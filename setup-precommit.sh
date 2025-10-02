#!/bin/bash
# Setup script für pre-commit hooks

echo "Installing pre-commit hooks for ESP32 project..."

# Install pre-commit if not available
if ! command -v pre-commit &> /dev/null; then
    echo "Installing pre-commit..."
    pip install pre-commit
fi

# Install platformio if not available (for compile check)
if ! command -v platformio &> /dev/null; then
    echo "PlatformIO not found! Please install it first."
    exit 1
fi

# Setup clang-format config
cat > .clang-format << EOF
BasedOnStyle: Google
IndentWidth: 2
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
EOF

# Setup secrets baseline
echo "Creating secrets baseline..."
detect-secrets scan --baseline .secrets.baseline

# Install hooks
echo "Installing pre-commit hooks..."
pre-commit install

echo "Testing hooks on all files..."
pre-commit run --all-files

echo "Setup complete! Pre-commit hooks are now active."
