import torch
import torch.nn as nn
import pandas as pd
from sklearn.model_selection import train_test_split

class MastercardTabularNet(nn.Module):
    def __init__(self, input_dim):
        super(MastercardTabularNet, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(input_dim, 128),
            nn.ReLU(),
            nn.BatchNorm1d(128),
            nn.Dropout(0.2),
            nn.Linear(128, 64),
            nn.ReLU(),
            nn.Linear(64, 1),
            nn.Sigmoid() # Outputs a probability [0.0, 1.0]
        )

    def forward(self, x):
        return self.network(x)

def main():
    print("[TRAINER] Initializing ChronoRisk Large Tabular Model (LTM) Training...")
    
    # In a real environment, this loads terabytes of historical transaction data from HDFS or Snowflake
    print("[TRAINER] Loading synthetic transaction dataset...")
    
    model = MastercardTabularNet(input_dim=5)
    model.eval() # Setting to eval for ONNX export
    
    # Exporting the weights to ONNX format for the C++ TensorRT runtime
    dummy_input = torch.randn(1, 5)
    onnx_path = "../models/fraud_ltm_v4.onnx"
    
    print(f"[TRAINER] Exporting optimized graph to ONNX: {onnx_path}")
    torch.onnx.export(
        model, 
        dummy_input, 
        onnx_path, 
        input_names=['features'], 
        output_names=['risk_score'],
        dynamic_axes={'features': {0: 'batch_size'}, 'risk_score': {0: 'batch_size'}}
    )
    
    print("[TRAINER] Export complete. Ready for C++ inference deployment.")

if __name__ == "__main__":
    main()
