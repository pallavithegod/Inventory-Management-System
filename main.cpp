#include <algorithm>  //for swapping, sorting
#include <cctype>     //isalpha, isdigit, isalnum, isspace
#include <fstream>    //file handling
#include <iomanip>    //setw, setprecision etc for spacing
#include <iostream>   //input output
#include <limits>     //int max, int min
#include <sstream>    //str to object conversion
#include <string>     //string operations
#include <vector>     //dynamic arrays

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

//formatting
string repeatPattern(const string& pattern, size_t repeatCount) {
    return string().append(repeatCount, ' ')
                   .replace(0, pattern.size() * repeatCount, string(repeatCount, ' '))
                   .assign(pattern.size() * repeatCount, pattern[0] == '+' ? '+' : pattern[0]);
}

void printSectionTitle(const string& title) {
    const auto border = string("+-").append(kTitleRepeat * 2, '-');
    cout << "\n" << border << ' ' << title << ' ' << border << "\n\n";
}

void printChipTableHeader() {
    const auto border = string(kBorderRepeat * 2, '-');
    cout << border << '\n';
    cout << left
         << setw(kIdWidth) << "Product ID"
         << setw(kNameWidth) << "Product Name"
         << setw(kQuantityWidth) << "Quantity"
         << setw(kSellerWidth) << "Seller Name"
         << setw(kPriceWidth) << "Price"
         << setw(kBrandWidth) << "Brand Name"
         << setw(kDeadstockWidth) << "Deadstock" << '\n';
    cout << border << '\n';
}

void printChipTableRow(const Chip& chip) {
    cout << left
         << setw(kIdWidth) << chip.productId
         << setw(kNameWidth) << chip.productName
         << setw(kQuantityWidth) << chip.quantity
         << setw(kSellerWidth) << chip.sellerName
         << setw(kPriceWidth) << chip.price
         << setw(kBrandWidth) << chip.brandName
         << setw(kDeadstockWidth) << chip.deadstock << '\n';
}

void printChipTableFooter(size_t recordCount) {
    cout << '\n' << " TOTAL RECORDS : " << recordCount << " \n\n";
}

bool promptYesNo(const string& prompt) {
    while (true) {
        cout << prompt;
        string input; getline(cin, input);
        if (input.empty()) { cout << "Please respond with Y or N.\n"; continue; }
        char r = tolower(input[0]);
        if (r == 'y') return true;
        if (r == 'n') return false;
        cout << "Please respond with Y or N.\n";
    }
}

bool parseInt(const string& s, int& v) {
    try { size_t pos; v = stoi(s, &pos); return pos == s.size(); }
    catch (...) { return false; }
}

int promptInt(const string& msg) {
    while (true) {
        cout << msg;
        string s; getline(cin, s);
        int v; if (parseInt(s, v)) return v;
        cout << "Invalid number. Please try again.\n";
    }
}

int promptOptionalInt(const string& msg, int cur) {
    cout << msg; string s; getline(cin, s);
    int v; return s.empty() ? cur : (parseInt(s, v) ? v : promptOptionalInt(msg, cur));
}

string promptString(const string& msg) {
    cout << msg; string s; getline(cin, s); return s;
}

string promptOptionalString(const string& msg, const string& cur) {
    cout << msg; string s; getline(cin, s); return s.empty() ? cur : s;
}

vector<string> tokenize(const string& line) {
    vector<string> t; string x; istringstream ss(line);
    while (getline(ss, x, ',')) t.push_back(x);
    if (!line.empty() && line.back() == ',') t.emplace_back();
    return t;
}

Chip chipFromTokens(const vector<string>& t) {
    Chip c; if (t.size() < 6) throw runtime_error("Insufficient columns");
    parseInt(t[0], c.productId);
    c.productName = t[1];
    parseInt(t[2], c.quantity);
    c.sellerName = t[3];
    parseInt(t[4], c.price);
    c.brandName = t[5];
    c.deadstock = (t.size() >= 7 && parseInt(t[6], c.deadstock)) ? c.deadstock : 0;
    return c;
}

string toCsvRow(const Chip& c) {
    ostringstream o;
    o << c.productId << ',' << c.productName << ',' << c.quantity << ','
      << c.sellerName << ',' << c.price << ',' << c.brandName << ',' << c.deadstock;
    return o.str();
}

void loadChips(vector<Chip>& v) {
    v.clear(); ifstream in(kDataFile);
    if (!in.is_open()) { ofstream f(kDataFile); return; }
    string line; size_t n = 0;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try { v.push_back(chipFromTokens(tokenize(line))); }
        catch (const exception& e) { cerr << "Skipping line " << ++n << ": " << e.what() << '\n'; }
    }
}

void saveChips(const vector<Chip>& v) {
    ofstream o(kDataFile, ios::trunc);
    for (auto& c : v) o << toCsvRow(c) << '\n';
}

void showAll(const vector<Chip>& v) {
    if (v.empty()) {
        cout << string(kTitleRepeat * 2, '-') << '\n'
             << "## Inventory is empty. Use the ADD option to register products.\n"
             << string(kTitleRepeat * 2, '-') << '\n';
        return;
    }
    printSectionTitle("CHIP INVENTORY SNAPSHOT");
    printChipTableHeader();
    for (auto& c : v) printChipTableRow(c);
    cout << string(kBorderRepeat * 2, '-') << '\n';
    printChipTableFooter(v.size());
}

Chip* findChip(vector<Chip>& v, int id) {
    auto it = find_if(v.begin(), v.end(), [id](const Chip& c) { return c.productId == id; });
    return it == v.end() ? nullptr : &(*it);
}

void addChip(vector<Chip>& v) {
    printSectionTitle("ADD NEW PRODUCT");
    Chip c;
    while (true) {
        c.productId = promptInt("Enter Product ID: ");
        if (!findChip(v, c.productId)) break;
        cout << "Product ID already exists. Enter unique ID.\n";
    }
    c.productName = promptString("Enter Product Name: ");
    c.quantity = promptInt("Enter Quantity: ");
    c.sellerName = promptString("Enter Seller Name: ");
    c.price = promptInt("Enter Price (per unit): ");
    c.brandName = promptString("Enter Brand Name: ");
    c.deadstock = promptInt("Enter Deadstock (0 if none): ");
    v.push_back(c); saveChips(v);
    cout << "\n## RECORD ADDED SUCCESSFULLY!\n\n";
}

void searchChip(vector<Chip>& v) {
    printSectionTitle("SEARCH PRODUCT FORM");
    if (v.empty()) { cout << "Inventory empty. Add products first.\n"; return; }
    int id = promptInt("Enter Product ID to search: ");
    Chip* c = findChip(v, id);
    if (!c) { cout << "## NO MATCH FOUND ##\n\n"; return; }
    printChipTableHeader(); printChipTableRow(*c);
    cout << string(kBorderRepeat * 2, '-') << '\n';
}

void editChip(vector<Chip>& v) {
    printSectionTitle("EDIT PRODUCT DETAILS");
    if (v.empty()) { cout << "Inventory empty.\n"; return; }
    int id = promptInt("Enter Product ID to edit: ");
    Chip* c = findChip(v, id);
    if (!c) { cout << "## NO MATCH FOUND ##\n\n"; return; }

    printChipTableHeader(); printChipTableRow(*c);
    cout << string(kBorderRepeat * 2, '-') << '\n';
    if (!promptYesNo("Proceed to update? (y/n): ")) return;

    cout << "Leave blank to keep current value.\n";
    c->productName = promptOptionalString("Product Name [" + c->productName + "]: ", c->productName);
    c->quantity = promptOptionalInt("Quantity [" + to_string(c->quantity) + "]: ", c->quantity);
    c->sellerName = promptOptionalString("Seller Name [" + c->sellerName + "]: ", c->sellerName);
    c->price = promptOptionalInt("Price [" + to_string(c->price) + "]: ", c->price);
    c->brandName = promptOptionalString("Brand [" + c->brandName + "]: ", c->brandName);
    c->deadstock = promptOptionalInt("Deadstock [" + to_string(c->deadstock) + "]: ", c->deadstock);
    saveChips(v); cout << "\n## RECORD UPDATED ##\n\n";
}

void deleteChip(vector<Chip>& v) {
    printSectionTitle("DELETE PRODUCT DETAILS");
    if (v.empty()) { cout << "Inventory empty.\n"; return; }
    int id = promptInt("Enter Product ID to delete: ");
    Chip* c = findChip(v, id);
    if (!c) { cout << "## NO MATCH FOUND ##\n\n"; return; }

    printChipTableHeader(); printChipTableRow(*c);
    cout << string(kBorderRepeat * 2, '-') << '\n';
    if (!promptYesNo("Confirm delete? (y/n): ")) return;

    v.erase(remove_if(v.begin(), v.end(), [id](const Chip& x){return x.productId == id;}), v.end());
    saveChips(v); cout << "\n## RECORD DELETED ##\n\n";
}

void contactInfo() {
    printSectionTitle("CONTACT US");
    const auto border = string(60, '*');
    cout << border << '\n'
         << setw(15) << " " << "Support Desk : OOPS Supply Co.\n"
         << setw(15) << " " << "Email        : oops.group8@bpit.com\n"
         << setw(15) << " " << "Phone        : 1234567890\n"
         << setw(15) << " " << "Hours        : Mon-Sat 9:00-18:00\n"
         << border << "\n\n";
}

void generateBill(vector<Chip>& v) {
    printSectionTitle("BILL SLIP");
    if (v.empty()) { cout << "Inventory empty.\n"; return; }
    int id = promptInt("\nEnter Product ID to bill: ");
    Chip* c = findChip(v, id);
    if (!c) { cout << "## NO MATCH FOUND ##\n\n"; return; }
    if (c->quantity == 0) { cout << "Out of stock.\n"; return; }

    int qty = 0;
    while (true) {
        qty = promptInt("Enter quantity to purchase: ");
        if (qty <= 0) cout << "Must be > 0.\n";
        else if (qty > c->quantity) cout << "Insufficient stock. Available: " << c->quantity << ".\n";
        else break;
    }

    double unit = c->price, subtotal = unit * qty, gst = subtotal * kDefaultGstRate;
    double mrp = subtotal + gst, disc = (mrp <= 1000) ? 0.05 : (unit < 3000 ? 0.08 : 0.12);
    double net = mrp * (1 - disc), save = mrp * disc;

    cout << fixed << setprecision(2);
    cout << "\n================================ BILL DETAILS ================================\n";
    cout << "PRODUCT ID   : " << c->productId << "   PRODUCT NAME : " << c->productName << '\n'
         << "SELLER NAME  : " << c->sellerName << "\nBRAND NAME   : " << c->brandName << '\n';
    cout << "UNITS        : " << qty << "\nUNIT PRICE   : Rs. " << unit
         << "\nSUBTOTAL     : Rs. " << subtotal << "\nGST (18%)    : Rs. " << gst
         << "\nTOTAL (incl GST): Rs. " << mrp << "\nDISCOUNT     : Rs. -" << save
         << "\n-----------------------------------------------\n"
         << "NET AMOUNT   : Rs. " << net << "\nYOU SAVED    : Rs. " << save << '\n';
    cout << setw(25) << " " << "THANK YOU FOR YOUR VISIT!\n\n";

    c->quantity -= qty; saveChips(v);
    cout << "Inventory updated.\n";
}

void menuLoop() {
    vector<Chip> v;
    try { loadChips(v); }
    catch (const exception& e) { cerr << "Init error: " << e.what() << '\n'; return; }

    while (true) {
        const auto border = string(58, '=');
        cout << '\n' << border << '\n' << setw(10) << " " << "CHIP INVENTORY MANAGEMENT MENU\n"
             << border << '\n'
             << "1. SHOW PRODUCT DETAILS\n2. ADD NEW PRODUCT\n3. SEARCH PRODUCT\n4. EDIT PRODUCT DETAILS\n"
             << "5. DELETE PRODUCT\n6. GENERATE BILL\n7. CONTACT SUPPORT\n0. EXIT\n"
             << border << '\n' << "Enter your choice: ";

        string s; getline(cin, s); int ch;
        if (!parseInt(s, ch)) { cout << "Invalid input.\n"; continue; }

        switch (ch) {
            case 1: showAll(v); break;
            case 2: addChip(v); break;
            case 3: searchChip(v); break;
            case 4: editChip(v); break;
            case 5: deleteChip(v); break;
            case 6: generateBill(v); break;
            case 7: contactInfo(); break;
            case 0: cout << "\nGOODBYE!!\n"; return;
            default: cout << "Enter between 0-7.\n";
        }
    }
}

int main() {
    try { menuLoop(); }
    catch (const exception& e) { cerr << "Fatal error: " << e.what() << '\n'; return 1; }
    return 0;
}
