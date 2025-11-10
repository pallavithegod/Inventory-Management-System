#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

struct Chip {
    int productId{};
    std::string productName;
    int quantity{};
    std::string sellerName;
    int price{};
    std::string brandName;
    int deadstock{};
};

namespace {

constexpr char kDataFile[] = "chips.csv";
constexpr double kDefaultGstRate = 0.18;
constexpr std::size_t kTableWidth = 110;
constexpr std::size_t kBorderRepeat = kTableWidth / 2;
constexpr std::size_t kTitleRepeat = 24;

constexpr int kIdWidth = 12;
constexpr int kNameWidth = 24;
constexpr int kQuantityWidth = 10;
constexpr int kSellerWidth = 26;
constexpr int kPriceWidth = 10;
constexpr int kBrandWidth = 18;
constexpr int kDeadstockWidth = 10;

std::string repeatPattern(const std::string& pattern, std::size_t repeatCount) {
    std::string result;
    result.reserve(pattern.size() * repeatCount);
    for (std::size_t i = 0; i < repeatCount; ++i) {
        result += pattern;
    }
    return result;
}

void printSectionTitle(const std::string& title) {
    const auto border = repeatPattern("+-", kTitleRepeat);
    std::cout << "\n" << border << ' ' << title << ' ' << border << "\n\n";
}

void printChipTableHeader() {
    const auto border = repeatPattern("--", kBorderRepeat);
    std::cout << border << '\n';
    std::cout << std::left
              << std::setw(kIdWidth) << "Product ID"
              << std::setw(kNameWidth) << "Product Name"
              << std::setw(kQuantityWidth) << "Quantity"
              << std::setw(kSellerWidth) << "Seller Name"
              << std::setw(kPriceWidth) << "Price"
              << std::setw(kBrandWidth) << "Brand Name"
              << std::setw(kDeadstockWidth) << "Deadstock"
              << '\n';
    std::cout << border << '\n';
}

void printChipTableRow(const Chip& chip) {
    std::cout << std::left
              << std::setw(kIdWidth) << chip.productId
              << std::setw(kNameWidth) << chip.productName
              << std::setw(kQuantityWidth) << chip.quantity
              << std::setw(kSellerWidth) << chip.sellerName
              << std::setw(kPriceWidth) << chip.price
              << std::setw(kBrandWidth) << chip.brandName
              << std::setw(kDeadstockWidth) << chip.deadstock
              << '\n';
}

void printChipTableFooter(std::size_t recordCount) {
    std::cout << '\n'
              << " TOTAL RECORDS : " << recordCount
              << ' ' << "\n\n";
}

bool promptYesNo(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string input;
        if (!std::getline(std::cin, input)) {
            throw std::runtime_error("Input stream closed unexpectedly.");
        }

        if (input.empty()) {
            std::cout << "Please respond with Y or N.\n";
            continue;
        }

        const char response = static_cast<char>(std::tolower(input.front()));
        if (response == 'y') {
            return true;
        }
        if (response == 'n') {
            return false;
        }
        std::cout << "Please respond with Y or N.\n";
    }
}

void contactInfo();

bool parseInt(const std::string& input, int& value) {
    std::istringstream iss(input);
    iss >> value;
    return !input.empty() && iss.eof() && !iss.fail();
}

int promptInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Input stream closed unexpectedly.");
        }

        int value{};
        if (parseInt(line, value)) {
            return value;
        }
        std::cout << "Invalid number. Please try again.\n";
    }
}

int promptOptionalInt(const std::string& prompt, int currentValue) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Input stream closed unexpectedly.");
        }

        if (line.empty()) {
            return currentValue;
        }

        int value{};
        if (parseInt(line, value)) {
            return value;
        }
        std::cout << "Invalid number. Please try again.\n";
    }
}

std::string promptString(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Input stream closed unexpectedly.");
    }
    return line;
}

std::string promptOptionalString(const std::string& prompt,
                                 const std::string& currentValue) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Input stream closed unexpectedly.");
    }
    return line.empty() ? currentValue : line;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(line);

    while (std::getline(iss, token, ',')) {
        tokens.emplace_back(std::move(token));
    }
    if (!line.empty() && line.back() == ',') {
        tokens.emplace_back();
    }
    return tokens;
}

Chip chipFromTokens(const std::vector<std::string>& tokens) {
    Chip chip;
    if (tokens.size() < 6) {
        throw std::runtime_error("Insufficient columns to parse chip.");
    }

    int value{};

    if (!parseInt(tokens[0], value)) {
        throw std::runtime_error("Invalid Product_ID value.");
    }
    chip.productId = value;

    chip.productName = tokens[1];

    if (!parseInt(tokens[2], value)) {
        throw std::runtime_error("Invalid Quantity value.");
    }
    chip.quantity = value;

    chip.sellerName = tokens[3];

    if (!parseInt(tokens[4], value)) {
        throw std::runtime_error("Invalid Price value.");
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

std::string toCsvRow(const Chip& chip) {
    std::ostringstream oss;
    oss << chip.productId << ','
        << chip.productName << ','
        << chip.quantity << ','
        << chip.sellerName << ','
        << chip.price << ','
        << chip.brandName << ','
        << chip.deadstock;
    return oss.str();
}

void loadChips(std::vector<Chip>& chips) {
    chips.clear();
    std::ifstream input(kDataFile);
    if (!input.is_open()) {
        std::ofstream createFile(kDataFile);
        if (!createFile.is_open()) {
            throw std::runtime_error("Unable to create data file.");
        }
        return;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }

        try {
            const auto tokens = tokenize(line);
            chips.emplace_back(chipFromTokens(tokens));
        } catch (const std::exception& ex) {
            std::cerr << "Skipping malformed line " << lineNumber << ": "
                      << ex.what() << '\n';
        }
    }
}

void saveChips(const std::vector<Chip>& chips) {
    std::ofstream output(kDataFile, std::ios::trunc);
    if (!output.is_open()) {
        throw std::runtime_error("Failed to open data file for writing.");
    }

    for (const auto& chip : chips) {
        output << toCsvRow(chip) << '\n';
    }
}

void showAll(const std::vector<Chip>& chips) {
    if (chips.empty()) {
        std::cout << repeatPattern("--", kTitleRepeat * 2 + 1) << '\n';
        std::cout << "## Inventory is empty. Use the ADD option to register products.\n";
        std::cout << repeatPattern("--", kTitleRepeat * 2 + 1) << '\n';
        return;
    }

    printSectionTitle("CHIP INVENTORY SNAPSHOT");
    printChipTableHeader();
    for (const auto& chip : chips) {
        printChipTableRow(chip);
    }
    std::cout << repeatPattern("--", kBorderRepeat) << '\n';
    printChipTableFooter(chips.size());
}

Chip* findChip(std::vector<Chip>& chips, int productId) {
    auto it = std::find_if(chips.begin(), chips.end(),
                           [productId](const Chip& chip) {
                               return chip.productId == productId;
                           });
    if (it == chips.end()) {
        return nullptr;
    }
    return &(*it);
}

void addChip(std::vector<Chip>& chips) {
    printSectionTitle("ADD NEW PRODUCT");
    Chip chip;

    while (true) {
        chip.productId = promptInt("Enter Product ID: ");
        if (!findChip(chips, chip.productId)) {
            break;
        }
        std::cout << "Product ID already exists. Please enter a unique ID.\n";
    }

    chip.productName = promptString("Enter Product Name: ");
    chip.quantity = promptInt("Enter Quantity: ");
    chip.sellerName = promptString("Enter Seller Name: ");
    chip.price = promptInt("Enter Price (per unit): ");
    chip.brandName = promptString("Enter Brand Name: ");
    chip.deadstock = promptInt("Enter Deadstock (0 if none): ");

    chips.emplace_back(chip);
    saveChips(chips);
    std::cout << "\n## RECORD ADDED SUCCESSFULLY!\n\n";
}

void searchChip(std::vector<Chip>& chips) {
    printSectionTitle("SEARCH PRODUCT FORM");
    if (chips.empty()) {
        std::cout << "Inventory is empty. Add products before searching.\n";
        return;
    }

    int id = promptInt("Enter Product ID to search: ");
    Chip* chip = findChip(chips, id);

    if (!chip) {
        std::cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    std::cout << repeatPattern("--", kBorderRepeat) << '\n';
}

void editChip(std::vector<Chip>& chips) {
    printSectionTitle("EDIT PRODUCT DETAILS");
    if (chips.empty()) {
        std::cout << "Inventory is empty. Add products before editing.\n";
        return;
    }

    int id = promptInt("Enter Product ID to edit: ");
    Chip* chip = findChip(chips, id);

    if (!chip) {
        std::cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    std::cout << repeatPattern("--", kBorderRepeat) << '\n';

    if (!promptYesNo("Proceed to update this product? (y/n): ")) {
        std::cout << "Update cancelled.\n";
        return;
    }

    std::cout << "Leave a field blank to keep the current value.\n";
    chip->productName = promptOptionalString(
        "Product Name [" + chip->productName + "]: ", chip->productName);
    chip->quantity = promptOptionalInt(
        "Quantity [" + std::to_string(chip->quantity) + "]: ", chip->quantity);
    chip->sellerName = promptOptionalString(
        "Seller Name [" + chip->sellerName + "]: ", chip->sellerName);
    chip->price = promptOptionalInt(
        "Price [" + std::to_string(chip->price) + "]: ", chip->price);
    chip->brandName = promptOptionalString(
        "Brand Name [" + chip->brandName + "]: ", chip->brandName);
    chip->deadstock = promptOptionalInt(
        "Deadstock [" + std::to_string(chip->deadstock) + "]: ",
        chip->deadstock);

    saveChips(chips);
    std::cout << "\n## RECORD UPDATED ##\n\n";
}

void deleteChip(std::vector<Chip>& chips) {
    printSectionTitle("DELETE PRODUCT DETAILS");
    if (chips.empty()) {
        std::cout << "Inventory is empty. Add products before deleting.\n";
        return;
    }

    int id = promptInt("Enter Product ID to delete: ");
    Chip* chip = findChip(chips, id);
    if (!chip) {
        std::cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    printChipTableHeader();
    printChipTableRow(*chip);
    std::cout << repeatPattern("--", kBorderRepeat) << '\n';

    if (!promptYesNo("Are you sure you want to delete this product? (y/n): ")) {
        std::cout << "Deletion cancelled.\n";
        return;
    }

    auto it = std::remove_if(chips.begin(), chips.end(),
                             [id](const Chip& candidate) {
                                 return candidate.productId == id;
                             });
    chips.erase(it, chips.end());
    saveChips(chips);
    std::cout << "\n## RECORD DELETED ##\n\n";
}

void generateBill(std::vector<Chip>& chips) {
    printSectionTitle("BILL SLIP");
    if (chips.empty()) {
        std::cout << "Inventory is empty. Add products before billing.\n";
        return;
    }

    int id = promptInt("\nEnter Product ID to bill: ");
    Chip* chip = findChip(chips, id);
    if (!chip) {
        std::cout << "## SORRY! NO MATCHING DETAILS AVAILABLE ##\n\n";
        return;
    }

    if (chip->quantity == 0) {
        std::cout << "Product is out of stock.\n";
        return;
    }

    int purchaseQty = 0;
    while (true) {
        purchaseQty = promptInt("Enter quantity to purchase: ");
        if (purchaseQty <= 0) {
            std::cout << "Quantity must be greater than zero.\n";
        } else if (purchaseQty > chip->quantity) {
            std::cout << "Insufficient stock. Available quantity: "
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
    std::cout << '\n' << border << " BILL DETAILS " << border << "\n\n";
    std::cout << starBorder << '\n';
    std::cout << "PRODUCT ID   : " << chip->productId
              << std::setw(18) << " "
              << "PRODUCT NAME : " << chip->productName << '\n';
    std::cout << "SELLER NAME  : " << chip->sellerName << '\n';
    std::cout << "BRAND NAME   : " << chip->brandName << '\n';
    std::cout << starBorder << '\n';

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "UNITS        : " << purchaseQty << '\n';
    std::cout << "UNIT PRICE   : Rs. " << unitPrice << '\n';
    std::cout << "SUBTOTAL     : Rs. " << subtotal << '\n';
    std::cout << "GST @ " << static_cast<int>(kDefaultGstRate * 100)
              << "%    : Rs. " << gstAmount << '\n';
    std::cout << "MRP (incl. GST): Rs. " << totalMrp << '\n';
    std::cout << "DISCOUNT @" << discountRate * 100
              << "% : Rs. -" << discountAmount << '\n';
    std::cout << repeatPattern("-=", 36) << '\n';
    std::cout << "NET AMOUNT   : Rs. " << netAmount << '\n';
    std::cout << "YOU SAVED    : Rs. " << savings << '\n';
    std::cout << std::defaultfloat << std::setprecision(6);
    std::cout << '\n' << starBorder << '\n';
    std::cout << std::setw(25) << " " << "THANK YOU FOR YOUR VISIT!\n";
    std::cout << starBorder << "\n\n";

    chip->quantity -= purchaseQty;
    saveChips(chips);
    std::cout << "Inventory updated.\n";
}

void menuLoop() {
    std::vector<Chip> chips;
    try {
        loadChips(chips);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to initialize data: " << ex.what() << '\n';
        return;
    }

    while (true) {
        const auto menuBorder = repeatPattern("=", 58);
        std::cout << '\n' << menuBorder << '\n';
        std::cout << std::setw(10) << " " << "CHIP INVENTORY MANAGEMENT MENU\n";
        std::cout << menuBorder << '\n';
        std::cout << "1. SHOW PRODUCT DETAILS\n";
        std::cout << "2. ADD NEW PRODUCT\n";
        std::cout << "3. SEARCH PRODUCT\n";
        std::cout << "4. EDIT PRODUCT DETAILS\n";
        std::cout << "5. DELETE PRODUCT\n";
        std::cout << "6. GENERATE BILL\n";
        std::cout << "7. CONTACT SUPPORT\n";
        std::cout << "0. EXIT\n";
        std::cout << menuBorder << '\n';
        std::cout << "Enter your choice: ";

        std::string choiceLine;
        if (!std::getline(std::cin, choiceLine)) {
            std::cout << "\nInput terminated. Exiting...\n";
            break;
        }

        int choice{};
        if (!parseInt(choiceLine, choice)) {
            std::cout << "Invalid option. Please enter a number between 0 and 7.\n";
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
                    std::cout << "\nGOODBYE!!\n";
                    break;
                default:
                    std::cout << "Invalid option. Please choose between 0 and 7.\n";
                    continue;
            }

            if (choice == 0) {
                return;
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << '\n';
        }
    }
}

void contactInfo() {
    printSectionTitle("CONTACT US");
    const auto border = repeatPattern("*", 60);
    std::cout << border << '\n';
    std::cout << std::setw(15) << " " << "Support Desk : Silicon Supply Co.\n";
    std::cout << std::setw(15) << " " << "Email        : support@siliconsupply.com\n";
    std::cout << std::setw(15) << " " << "Phone        : +1-800-555-CHIP\n";
    std::cout << std::setw(15) << " " << "Hours        : Mon-Sat 9:00-18:00\n";
    std::cout << border << "\n\n";
}

}  // namespace

int main() {
    try {
        menuLoop();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

