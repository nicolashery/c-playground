# Bean

Small plain-text accounting CLI tool using a minimal subset of [Beancount](https://github.com/beancount/beancount). Parses a ledger file, checks for invariants, and prints account balance report.

## Usage

**NOTE**: All commands assume running from root of repository.

After building (`task:configure && task build:bean`):

```bash
alias bean=./build/bean/bean

# Show usage
bean --help

# Check ledger file
bean check bean/data/basic.bean

# Print balance report
bean balance bean/data/basic.bean
```

Run end-to-end tests with:

```bash
./bean/test.sh
```

## File structure

Source files:

- `ledger.h/c`: Core data types, arrays, validation, printer, I/O
- `scanner.h/c`: Tokenization
- `parser.h/c`: Parsing
- `commands.h/c`: Commands (check, balance)
- `main.c`: Entry point and CLI routing

Other:

- `data/`: Sample Beancount ledgers
- `test/`: End-to-end test inputs and golden files
- `test.sh`: Test script

## Beancount subset

This example uses a minimal subset of the [Beancount Language Syntax](https://beancount.github.io/docs/beancount_language_syntax.html).

Only 3 directives supported:

- `commodity`
- `open`
- `transaction`

Comments are supported (start with `;` and continue to end of line).

**Commodity** or **currency** declaration:

```
2014-01-01 commodity USD
```

**NOTES**:
- Only a single currency per ledger is supported
- Currency name must be all capitals

**Account** opening:

```
2014-01-01 open Assets:Checking
2014-01-01 open Expenses:Food
```

**NOTES**:
- Accounts must be declared before use
- Account name is a colon-separated list of capitalized words which begin with a letter
- First word of account name bust be one of the 5 account types: Assets, Liabilities, Equity, Income, Expenses
- No date-based validation (account opening vs posting date)

**Transaction**:

```
2014-01-05 * "Groceries"
  Expenses:Food       45.12 USD
  Assets:Checking    -45.12 USD

2014-01-10 txn "Coffee Shop" "Morning coffee"
  Expenses:Food        4.50 USD
  Assets:Checking     -4.50 USD
```

**NOTES**:
- 3 flag formats: `txn` keyword (empty flag), `*` (cleared), `!` (warning)
- Optional payee string before narration string
- All postings must have an amount
- Postings balance to zero
- Amount format: fixed-point with exactly 2 decimal places
