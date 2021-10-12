#ifndef WALLET_H_
#define WALLET_H_

#include <string>
#include <vector>

class Wallet {

public:

	class Table {

	public:

		class Entry {

		private:

			std::string username;
			std::string domain;
			std::string password;
			std::string date;
			std::string description;

		public:

			explicit Entry();
			~Entry();
			std::string getUsername();
			std::string getDomain();
			std::string getPassword();
			std::string getDate();
			std::string getDescription();
			void setUsername(std::string username);
			void setDomain(std::string domain);
			void setPassword(std::string password);
			void setDate(std::string date);
			void setDescription(std::string description);

			// Compare classes
			bool operator==(const Entry& entry) const {
				return
				username == entry.username
				&&domain == entry.domain
				&&password == entry.password
				&&date == entry.date
				&&description == entry.description
				;
			}

			// Serialize fields by passing them to the archive
			template<class Archive>
			void serialize(Archive &archive) {
				archive(username, domain, password, date, description);
			}

		};

	private:

		std::string name;
		std::vector<Entry> entries;

	public:

		explicit Table();
		~Table();
		std::string getName();
		std::vector<Entry> getEntries();
		void setName(std::string name);
		void setEntries(std::vector<Entry> entries);
		void insertEntry(Wallet::Table::Entry entry);

		// Compare classes
		bool operator==(const Table& table) const {
			return
			entries == table.entries
			;
		}

		// Serialize fields by passing them to the archive
		template<class Archive>
		void serialize(Archive &archive) {
			archive(name, entries);
		}

	};

private:

	std::string path;
	std::string fileName;
	std::vector<Table> tables;

public:

	explicit Wallet();
	~Wallet();
	std::string getPath();
	std::string getFileName();
	std::vector<Table> getTables();
	void setPath(std::string path);
	void setFileName(std::string fileName);
	void setTables(std::vector<Table> tables);
	void insertTable(Wallet::Table table);

	// Compare classes
	bool operator==(const Wallet& wallet) const {
		return
		tables == wallet.tables
		;
	}

	// Serialize fields by passing them to the archive
	template<class Archive>
	void serialize(Archive &archive) {
		archive(path, fileName, tables);
	}

};

#endif /* WALLET_H_ */
