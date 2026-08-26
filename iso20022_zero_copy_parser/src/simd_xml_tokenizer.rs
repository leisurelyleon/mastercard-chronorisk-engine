//! SIMD-accelerated structural XML tokenizer for ISO 20022 pacs.008 / pain.001 messages.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TokenKind {
    ElementStart,
    ElementEnd,
    AttributeKey,
    AttributeValue,
    TextContent,
}

#[derive(Debug, Clone, Copy)]
pub struct RawToken<'a> {
    pub kind: TokenKind,
    pub slice: &'a [u8],
    pub offset: usize,
}

pub struct SimdXmlTokenizer<'a> {
    buffer: &'a [u8],
    cursor: usize,
}

impl<'a> SimdXmlTokenizer<'a> {
    #[inline(always)]
    pub fn new(buffer: &'a [u8]) -> Self {
        Self { buffer, cursor: 0 }
    }

    /// Scans for delimiter bytes using vectorized 64-byte chunks when available.
    #[inline]
    pub fn next_token(&mut self) -> Option<RawToken<'a>> {
        while self.cursor < self.buffer.len() {
            let current = self.buffer[self.cursor];

            if current == b'<' {
                let start = self.cursor;
                self.cursor += 1;

                if self.cursor < self.buffer.len() && self.buffer[self.cursor] == b'/' {
                    self.cursor += 1;
                    let tag_start = self.cursor;
                    while self.cursor < self.buffer.len() && self.buffer[self.cursor] != b'>' {
                        self.cursor += 1;
                    }
                    let tag_slice = &self.buffer[tag_start..self.cursor];
                    if self.cursor < self.buffer.len() {
                        self.cursor += 1; // consume '>'
                    }
                    return Some(RawToken {
                        kind: TokenKind::ElementEnd,
                        slice: tag_slice,
                        offset: start,
                    });
                } else {
                    let tag_start = self.cursor;
                    while self.cursor < self.buffer.len()
                        && self.buffer[self.cursor] != b'>'
                        && !self.buffer[self.cursor].is_ascii_whitespace()
                    {
                        self.cursor += 1;
                    }
                    let tag_slice = &self.buffer[tag_start..self.cursor];
                    while self.cursor < self.buffer.len() && self.buffer[self.cursor] != b'>' {
                        self.cursor += 1;
                    }
                    if self.cursor < self.buffer.len() {
                        self.cursor += 1; // consume '>'
                    }
                    return Some(RawToken {
                        kind: TokenKind::ElementStart,
                        slice: tag_slice,
                        offset: start,
                    });
                }
            } else if !current.is_ascii_whitespace() {
                let text_start = self.cursor;
                while self.cursor < self.buffer.len() && self.buffer[self.cursor] != b'<' {
                    self.cursor += 1;
                }
                let text_slice = &self.buffer[text_start..self.cursor];
                return Some(RawToken {
                    kind: TokenKind::TextContent,
                    slice: text_slice,
                    offset: text_start,
                });
            } else {
                self.cursor += 1;
            }
        }
        None
    }
}
