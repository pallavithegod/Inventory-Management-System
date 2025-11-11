#include <algorithm>  //for swapping, sorting
#include <cctype>   //isalpha, isdigit, isalnum, isspace
#include <fstream>  //file handling
#include <iomanip>  //setw, setprecision etc for spacing
#include <iostream>  //input output
#include <limits>    //int max, int min
#include <sstream>   //str to object conversion
#include <string>   //string operations
#include <vector>   //dynamic arrays

using namespace std;

struct Chip {
    int productId{};
    string productName;
    int quantity{};
    string sellerName;
    int price{};
    string brandName;
    int deadstock{};
};

namespace {

constexpr char kDataFile[] = "chips.csv";
constexpr double kDefaultGstRate = 0.18;
constexpr size_t kTableWidth = 110;
constexpr size_t kBorderRepeat = kTableWidth / 2;
constexpr size_t kTitleRepeat = 24;

constexpr int kIdWidth = 12;
constexpr int kNameWidth = 24;
constexpr int kQuantityWidth = 10;
constexpr int kSellerWidth = 26;
constexpr int kPriceWidth = 10;
constexpr int kBrandWidth = 18;
constexpr int kDeadstockWidth = 10;

string repeatPattern(const string& pattern, size_t repeatCount) {  //formatting
    string result;
    result.reserve(pattern.size() * repeatCount);    //space reserved beforhand
    for (size_t i = 0; i < repeatCount; ++i) {
        result += pattern;
    }
    return result;
}

void printSectionTitle(const string& title) {
    const auto border = repeatPattern("+-", kTitleRepeat);
    cout << "\n" << border << ' ' << title << ' ' << border << "\n\n";
}

void printChipTableHeader() {
    const auto border = repeatPattern("--", kBorderRepeat);
    cout << border << '\n';
    cout << left   //left align all columns
              << setw(kIdWidth) << "Product ID"
              << setw(kNameWidth) << "Product Name"
              << setw(kQuantityWidth) << "Quantity"
              << setw(kSellerWidth) << "Seller Name"
              << setw(kPriceWidth) << "Price"
              << setw(kBrandWidth) << "Brand Name"
              << setw(kDeadstockWidth) << "Deadstock"
              << '\n';
    cout << border << '\n';
}

void printChipTableRow(const Chip& chip) {   //definiton in structure
    cout << left
              << setw(kIdWidth) << chip.productId
              << setw(kNameWidth) << chip.productName
              << setw(kQuantityWidth) << chip.quantity
              << setw(kSellerWidth) << chip.sellerName
              << setw(kPriceWidth) << chip.price
              << setw(kBrandWidth) << chip.brandName
              << setw(kDeadstockWidth) << chip.deadstock
              << '\n';
}

void printChipTableFooter(size_t recordCount) {
    cout << '\n'
              << " TOTAL RECORDS : " << recordCount
              << ' ' << "\n\n";
}

bool promptYesNo(const string& prompt) {
    while (true) {
        cout << prompt;
        string input;
        if (!getline(cin, input)) {
            throw runtime_error("Input stream closed unexpectedly.");
        }

        if (input.empty()) {
            cout << "Please respond with Y or N.\n";
            continue;
        }

        const char response = static_cast<char>(tolower(input.front()));
        if (response == 'y') {
            return true;
        }
        if (response == 'n') {
            return false;
        }
        cout << "Please respond with Y or N.\n";
    }
}

void contactInfo();

bool parseInt(const string& input, int& value) {
    istringstream iss(input);
    iss >> value;
    return !input.empty() && iss.eof() && !iss.fail();
}

int promptInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) {
            throw runtime_error("Input stream closed unexpectedly.");
        }

        int value{};
        if (parseInt(line, value)) {
            return value;
        }
        cout << "Invalid number. Please try again.\n";
    }
}

int promptOptionalInt(const string& prompt, int currentValue) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) {
            throw runtime_error("Input stream closed unexpectedly.");
        }

        if (line.empty()) {
            return currentValue;
        }

        int value{};
        if (parseInt(line, value)) {
            return value;
        }
        cout << "Invalid number. Please try again.\n";
    }
}

string promptString(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        throw runtime_error("Input stream closed unexpectedly.");
    }
    return line;
}

string promptOptionalString(const string& prompt,
                                 const string& currentValue) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        throw runtime_error("Input stream closed unexpectedly.");
    }
    return line.empty() ? currentValue : line;
}

vector<string> tokenize(const string& line) {
    vector<string> tokens;
    string token;
    istringstream iss(line);

    while (getline(iss, token, ',')) {
        tokens.emplace_back(move(token));
    }
    if (!line.empty() && line.back() == ',') {
        tokens.emplace_back();
    }
    return tokens;
}

Chip chipFromTokens(const vector<string>& tokens) {
    Chip chip;
    if (tokens.size() < 6) {
        throw runtime_error("Insufficient columns to parse chip.");
    }

    int value{};

    if (!parseInt(tokens[0], value)) {
        throw runtime_error("Invalid Product_ID value.");
    }
    chip.productId = value;

    chip.productName = tokens[1];

    if (!parseInt(tokens[2], value)) {
        throw runtime_error("Invalid Quantity value.");
    }
    chip.quantity = value;

    chip.sellerName = tokens[3];

    if (!parseInt(tokens[4], value)) {
        throw runtime_error("Invalid Price value.");
    }
    chip.price = value;

    chip.brandName = tokens[5];

    if (tokens.size() >= 7 && parseInt(tokens[6], value)) {
        chip.deadstock = value;
    } else {
        chip.deadstock = 0;
    }

    return chip;
}

string toCsvRow(const Chip& chip) {
    ostringstream oss;
    oss << chip.productId << ','
        << chip.productName << ','
        << chip.quantity << ','
        << chip.sellerName << ','
        << chip.price << ','
        << chip.brandName << ','
        << chip.deadstock;
    return oss.str();
}

void loadChips(vector<Chip>& chips) {
    chips.clear();
    ifstream input(kDataFile);
    if (!input.is_open()) {
        ofstream createFile(kDataFile);
        if (!createFile.is_open()) {
            throw runtime_error("Unable to create data file.");
        }
        return;
    }

    string line;
    size_t lineNumber = 0;
    while (getline(input, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }

        try {
            const auto tokens = tokenize(line);
            chips.emplace_back(chipFromTokens(tokens));
        } catch (const exception& ex) {
            cerr << "Skipping malformed line " << lineNumber << ": "
                      << ex.what() << '\n';
        }
    }
}

void saveChips(const vector<Chip>& chips) {
    ofstream output(kDataFile, ios::trunc);
    if (!output.is_open()) {
        throw runtime_error("Failed to open data file for writing.");
    }

    for (const auto& chip : chips) {
        output << toCsvRow(chip) << '\n';
    }
}

void showAll(const vector<Chip>& chips) {
    if (chips.empty()) {
        cout << repeatPattern("--", kTitleRepeat * 2 + 1) << '\n';
        cout << "## Inventory is empty. Use the ADD option to register products.\n";
        cout << repeatPattern("--", kTitleRepeat * 2 + 1) << '\n';
        return;
    }

    printSectionTitle("CHIP INVENTORY SNAPSHOT");
    printChipTableHeader();
    for (const auto& chip : chips) {
        printChipTableRow(chip);
    }
    cout << repeatPattern("--", kBorderRepeat) << '\n';
    printChipTableFooter(chips.size());
}

Chip* findChip(vector<Chip>& chips, int productId) {
    auto it = find_if(chips.begin(), chips.end(),
                           [productId](const Chip& chip) {
                               return chip.productId == productId;
                           });
    if (it == chips.end()) {
        return nullptr;
    }
    return &(*it);
}

void addChip(vector<Chip>& chips) {
    printSectionTitle("ADD NEW PRODUCT");
    Chip chip;

    while (true) {
        chip.productId = promptInt("Enter Product ID: ");
        if (!findChip(chips, chip.productId)) {
            break;
        }
        cout << "Product ID already exists. Please enter a unique ID.\n";
    }

    chip.productName = promptString("Enter Product Name: ");
    chip.quantity = promptInt("Enter Quantity: ");
    chip.sellerName = promptString("Enter Seller Name: ");
    chip.price = promptInt("Enter Price (per unit): ");
    chip.brandName = promptString("Enter Brand Name: ");
    chip.deadstock = promptInt("Enter Deadstock (0 if none): ");

    chips.emplace_back(chip);
    saveChips(chips);
    cout << "\n## RECORD ADDED SUCCESSFULLY!\n\n";
}

void searchChip(vector<Chip>& chips) {
    printSectionTitle("SEARCH PRODUCT FORM");
    if (chips.empty()) {
        cout << "Inventory is empty. Add products before searching.\n";
        return;
    }

    int id = promptInt("Enter Product ID to search: ");
    Chip* chip = findChip(chips, id);

    if (!chip) {
        cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    cout << repeatPattern("--", kBorderRepeat) << '\n';
}

void editChip(vector<Chip>& chips) {
    printSectionTitle("EDIT PRODUCT DETAILS");
    if (chips.empty()) {
        cout << "Inventory is empty. Add products before editing.\n";
        return;
    }

    int id = promptInt("Enter Product ID to edit: ");
    Chip* chip = findChip(chips, id);

    if (!chip) {
        cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    cout << repeatPattern("--", kBorderRepeat) << '\n';

    if (!promptYesNo("Proceed to update this product? (y/n): ")) {
        cout << "Update cancelled.\n";
        return;
    }

    cout << "Leave a field blank to keep the current value.\n";
    chip->productName = promptOptionalString(
        "Product Name [" + chip->productName + "]: ", chip->productName);
    chip->quantity = promptOptionalInt(
        "Quantity [" + to_string(chip->quantity) + "]: ", chip->quantity);
    chip->sellerName = promptOptionalString(
        "Seller Name [" + chip->sellerName + "]: ", chip->sellerName);
    chip->price = promptOptionalInt(
        "Price [" + to_string(chip->price) + "]: ", chip->price);
    chip->brandName = promptOptionalString(
        "Brand Name [" + chip->brandName + "]: ", chip->brandName);
    chip->deadstock = promptOptionalInt(
        "Deadstock [" + to_string(chip->deadstock) + "]: ",
        chip->deadstock);

    saveChips(chips);
    cout << "\n## RECORD UPDATED ##\n\n";
}

void deleteChip(vector<Chip>& chips) {
    printSectionTitle("DELETE PRODUCT DETAILS");
    if (chips.empty()) {
        cout << "Inventory is empty. Add products before deleting.\n";
        return;
    }

    int id = promptInt("Enter Product ID to delete: ");
    Chip* chip = findChip(chips, id);
    if (!chip) {
        cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    cout << repeatPattern("--", kBorderRepeat) << '\n';

    if (!promptYesNo("Are you sure you want to delete this product? (y/n): ")) {
        cout << "Deletion cancelled.\n";
        return;
    }

    auto it = remove_if(chips.begin(), chips.end(),
                             [id](const Chip& candidate) {
                                 return candidate.productId == id;
                             });
    chips.erase(it, chips.end());
    saveChips(chips);
    cout << "\n## RECORD DELETED ##\n\n";
}

void generateBill(vector<Chip>& chips) {
    printSectionTitle("BILL SLIP");
    if (chips.empty()) {
        cout << "Inventory is empty. Add products before billing.\n";
        return;
    }

    int id = promptInt("\nEnter Product ID to bill: ");
    Chip* chip = findChip(chips, id);
    if (!chip) {
        cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    if (chip->quantity == 0) {
        cout << "Product is out of stock.\n";
        return;
    }

    int purchaseQty = 0;
    while (true) {
        purchaseQty = promptInt("Enter quantity to purchase: ");
        if (purchaseQty <= 0) {
            cout << "Quantity must be greater than zero.\n";
        } else if (purchaseQty > chip->quantity) {
            cout << "Insufficient stock. Available quantity: "
                      << chip->quantity << ".\n";
        } else {
            break;
        }
    }

    const double unitPrice = static_cast<double>(chip->price);
    const double subtotal = unitPrice * purchaseQty;
    const double gstAmount = subtotal * kDefaultGstRate;
    const double unitMrp = unitPrice * (1.0 + kDefaultGstRate);
    const double totalMrp = unitMrp * purchaseQty;

    double discountRate = 0.0;
    if (unitMrp <= 200.0) {
        discountRate = 0.05;
    } else if (unitPrice < 3000.0) {
        discountRate = 0.08;
    } else {
        discountRate = 0.12;
    }

    const double discountAmount = totalMrp * discountRate;
    const double netAmount = totalMrp - discountAmount;
    const double savings = discountAmount;

    const auto border = repeatPattern("--", 30);
    const auto starBorder = repeatPattern("*", 70);
    cout << '\n' << border << " BILL DETAILS " << border << "\n\n";
    cout << starBorder << '\n';
    cout << "PRODUCT ID   : " << chip->productId
              << setw(18) << " "
              << "PRODUCT NAME : " << chip->productName << '\n';
    cout << "SELLER NAME  : " << chip->sellerName << '\n';
    cout << "BRAND NAME   : " << chip->brandName << '\n';
    cout << starBorder << '\n';

    cout << fixed << setprecision(2);
    cout << "UNITS        : " << purchaseQty << '\n';
    cout << "UNIT PRICE   : Rs. " << unitPrice << '\n';
    cout << "SUBTOTAL     : Rs. " << subtotal << '\n';
    cout << "GST @ " << static_cast<int>(kDefaultGstRate * 100)
              << "%    : Rs. " << gstAmount << '\n';
    cout << "MRP (incl. GST): Rs. " << totalMrp << '\n';
    cout << "DISCOUNT @" << discountRate * 100
              << "% : Rs. -" << discountAmount << '\n';
    cout << repeatPattern("-=", 36) << '\n';
    cout << "NET AMOUNT   : Rs. " << netAmount << '\n';
    cout << "YOU SAVED    : Rs. " << savings << '\n';
    cout << defaultfloat << setprecision(6);
    cout << '\n' << starBorder << '\n';
    cout << setw(25) << " " << "THANK YOU FOR YOUR VISIT!\n";
    cout << starBorder << "\n\n";

    chip->quantity -= purchaseQty;
    saveChips(chips);
    cout << "Inventory updated.\n";
}

void menuLoop() {
    vector<Chip> chips;
    try {
        loadChips(chips);
    } catch (const exception& ex) {
        cerr << "Failed to initialize data: " << ex.what() << '\n';
        return;
    }

    while (true) {
        const auto menuBorder = repeatPattern("=", 58);
        cout << '\n' << menuBorder << '\n';
        cout << setw(10) << " " << "CHIP INVENTORY MANAGEMENT MENU\n";
        cout << menuBorder << '\n';
        cout << "1. SHOW PRODUCT DETAILS\n";
        cout << "2. ADD NEW PRODUCT\n";
        cout << "3. SEARCH PRODUCT\n";
        cout << "4. EDIT PRODUCT DETAILS\n";
        cout << "5. DELETE PRODUCT\n";
        cout << "6. GENERATE BILL\n";
        cout << "7. CONTACT SUPPORT\n";
        cout << "0. EXIT\n";
        cout << menuBorder << '\n';
        cout << "Enter your choice: ";

        string choiceLine;
        if (!getline(cin, choiceLine)) {
            cout << "\nInput terminated. Exiting...\n";
            break;
        }

        int choice{};
        if (!parseInt(choiceLine, choice)) {
            cout << "Invalid option. Please enter a number between 0 and 7.\n";
            continue;
        }

        try {
            switch (choice) {
                case 1:
                    showAll(chips);
                    break;
                case 2:
                    addChip(chips);
                    break;
                case 3:
                    searchChip(chips);
                    break;
                case 4:
                    editChip(chips);
                    break;
                case 5:
                    deleteChip(chips);
                    break;
                case 6:
                    generateBill(chips);
                    break;
                case 7:
                    contactInfo();
                    break;
                case 0:
                    cout << "\nGOODBYE!!\n";
                    break;
                default:
                    cout << "Invalid option. Please choose between 0 and 7.\n";
                    continue;
            }

            if (choice == 0) {
                return;
            }
        } catch (const exception& ex) {
            cerr << "Error: " << ex.what() << '\n';
        }
    }
}

void contactInfo() {
    printSectionTitle("CONTACT US");
    const auto border = repeatPattern("*", 60);
    cout << border << '\n';
    cout << setw(15) << " " << "Support Desk : OOPS Supply Co.\n";
    cout << setw(15) << " " << "Email        : oops.group8@bpit.com\n";
    cout << setw(15) << " " << "Phone        : 1234567890\n";
    cout << setw(15) << " " << "Hours        : Mon-Sat 9:00-18:00\n";
    cout << border << "\n\n";
}

}  // namespace

int main() {
    try {
        menuLoop();
    } catch (const exception& ex) {
        cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

