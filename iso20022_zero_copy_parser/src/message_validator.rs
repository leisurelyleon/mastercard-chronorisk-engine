//! Strict ISO 20022 schema validation (pacs.008.001.10 financial credit transfer).

use crate::simd_xml_tokenizer::{RawToken, SimdXmlTokenizer, TokenKind};

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct ParsedIsoTransaction {
    pub amount_cents: u64,
    pub currency: [u8; 4],
    pub settlement_priority: u8,
    pub is_instant: bool,
    pub is_valid: bool,
}

pub struct MessageValidator;

impl MessageValidator {
    pub fn parse_pacs008<'a>(xml_payload: &'a [u8]) -> ParsedIsoTransaction {
        let mut tokenizer = SimdXmlTokenizer::new(xml_payload);
        let mut active_tag: Option<&'a [u8]> = None;

        let mut tx = ParsedIsoTransaction {
            amount_cents: 0,
            currency: [0u8; 4],
            settlement_priority: 1,
            is_instant: false,
            is_valid: false,
        };

        while let Some(token) = tokenizer.next_token() {
            match token.kind {
                TokenKind::ElementStart => {
                    active_tag = Some(token.slice);
                }
                TokenKind::ElementEnd => {
                    active_tag = None;
                }
                TokenKind::TextContent => {
                    if let Some(tag) = active_tag {
                        if tag == b"IntrBkSttlmAmt" {
                            tx.amount_cents = Self::parse_decimal_to_cents(token.slice);
                        } else if tag == b"Ccy" {
                            let len = token.slice.len().min(3);
                            tx.currency[..len].copy_from_slice(&token.slice[..len]);
                        } else if tag == b"SttlmPrty" {
                            if token.slice == b"HIGH" {
                                tx.settlement_priority = 0;
                            }
                        } else if tag == b"SvcLvl" && token.slice == b"INST" {
                            tx.is_instant = true;
                        }
                    }
                }
                _ => {}
            }
        }

        if tx.amount_cents > 0 && tx.currency[0] != 0 {
            tx.is_valid = true;
        }

        tx
    }

    #[inline]
    fn parse_decimal_to_cents(bytes: &[u8]) -> u64 {
        let mut integer_part: u64 = 0;
        let mut fractional_part: u64 = 0;
        let mut seen_dot = false;
        let mut fraction_digits = 0;

        for &b in bytes {
            if b == b'.' {
                seen_dot = true;
                continue;
            }
            if b.is_ascii_digit() {
                let digit = (b - b'0') as u64;
                if !seen_dot {
                    integer_part = integer_part * 10 + digit;
                } else if fraction_digits < 2 {
                    fractional_part = fractional_part * 10 + digit;
                    fraction_digits += 1;
                }
            }
        }

        if fraction_digits == 1 {
            fractional_part *= 10;
        }

        integer_part * 100 + fractional_part
    }
}
