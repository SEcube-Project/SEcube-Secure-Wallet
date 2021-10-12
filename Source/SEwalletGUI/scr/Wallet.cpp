#include "Wallet.h"

/** Constructor
 *
 * @brief
 */
Wallet::Wallet() {
}

/** Destructor
 *
 * @brief Wallet::~Wallet
 */
Wallet::~Wallet() {

}

/** Get the wallet path
 *
 * @brief
 *
 * @return
 */
std::string Wallet::getPath() {
    return path;
}

/** Get the wallet fileName
 *
 * @brief
 *
 * @return
 */
std::string Wallet::getFileName() {
    return fileName;
}

/** Get the wallet tables
 *
 * @brief
 *
 * @return
 */
std::vector<Wallet::Table> Wallet::getTables() {
    return tables;
}

/** Insert a new table in the wallet
 *
 * @brief
 *
 * @param table
 */
void Wallet::insertTable(Wallet::Table table) {
    this->tables.insert(this->tables.end(),table);
}

/** Set the wallet path
 *
 * @brief
 *
 * @param path
 */
void Wallet::setPath(std::string path) {
    this->path = path;
}

/** Set the wallet fileName
 *
 * @brief
 *
 * @param fileName
 */
void Wallet::setFileName(std::string fileName) {
    this->fileName = fileName;
}

/** Set the wallet tables
 *
 * @brief
 *
 * @param tables
 */
void Wallet::setTables(std::vector<Table> tables) {
    this->tables = tables;
}

/** Constructor
 *
 * @brief
 */
Wallet::Table::Table() {
}

/** Destructor
 *
 * @brief Table::~Table
 */
Wallet::Table::~Table() {

}

/** Get the table name
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::getName() {
    return name;
}

/** Get the table entries
 *
 * @brief
 *
 * @return
 */
std::vector<Wallet::Table::Entry> Wallet::Table::getEntries() {
    return entries;
}

/** Set the table name
 *
 * @brief
 *
 * @param name
 */
void Wallet::Table::setName(std::string name) {
    this->name = name;
}

/** Set the table entries
 *
 * @brief
 *
 * @param entries
 */
void Wallet::Table::setEntries(std::vector<Entry> entries) {
    this->entries = entries;
}

/** Insert a new entry in the table
 *
 * @brief
 *
 * @param entry
 */
void Wallet::Table::insertEntry(Wallet::Table::Entry entry) {
    this->entries.insert(this->entries.end(), entry);
}

/** Constructor
 *
 * @brief
 */
Wallet::Table::Entry::Entry() {
}

/** Destructor
 *
 * @brief Entry::~Entry
 */
Wallet::Table::Entry::~Entry() {

}

/** Get the username field of entry
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::Entry::getUsername() {
    return username;
}

/** Get the domain field of entry
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::Entry::getDomain() {
    return domain;
}

/** Get the password field of entry
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::Entry::getPassword() {
    return password;
}

/** Get the date field of entry
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::Entry::getDate() {
    return date;
}

/** Get the description field of entry
 *
 * @brief
 *
 * @return
 */
std::string Wallet::Table::Entry::getDescription() {
    return description;
}

/** Set the username field of entry
 *
 * @brief
 *
 * @param username
 */
void Wallet::Table::Entry::setUsername(std::string username) {
    this->username = username;
}

/** Set the domain field of entry
 *
 * @brief
 *
 * @param domain
 */
void Wallet::Table::Entry::setDomain(std::string domain) {
    this->domain = domain;
}

/** Set the password field of entry
 *
 * @brief
 *
 * @param password
 */
void Wallet::Table::Entry::setPassword(std::string password) {
    this->password = password;
}

/** Set the date field of entry
 *
 * @brief
 *
 * @param date
 */
void Wallet::Table::Entry::setDate(std::string date) {
    this->date = date;
}

/** Set the description field of entry
 *
 * @brief
 *
 * @param description
 */
void Wallet::Table::Entry::setDescription(std::string description) {
    this->description = description;
}


