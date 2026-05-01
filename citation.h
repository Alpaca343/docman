#pragma once
#ifndef CITATION_H
#define CITATION_H

#include <string>
#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>


enum class CiteType {
	BOOK,
	WEBPAGE,
	ARTICLE,
};

class Citation {
public:
	CiteType type;
	std::string id;

	Citation(const std::string& id, CiteType type) : id(id), type(type) {}
	virtual void print(std::ostream& os) const = 0;

};

class Book :public Citation {
public:
	std::string ISBN;
	std::string author;
	std::string title;
	std::string publisher;
	std::string year;

	Book(const std::string& id, const std::string& ISBN) : Citation(id, CiteType::BOOK), ISBN{ ISBN } {}

	void search() {
		httplib::Client client{ API_ENDPOINT };
		auto result = client.Get("/isbn/" + encodeUriComponent(ISBN));
		if (result && result->status == 200) {
			nlohmann::json content = nlohmann::json::parse(result->body);
			author = content["author"];
			title = content["title"];
			publisher = content["publisher"];
			year = content["year"];
		}
		else {
			std::exit(1);
		}
	}


	void print(std::ostream& os) const {
		os << '[' << id << "] " << "book: " << author << ", " << title
			<< ", " << publisher << ", " << year << std::endl;
	}
};

class Webpage : public Citation {
public:
	std::string url;
	std::string webTitle;

	Webpage(const std::string& id, const std::string& url) : Citation(id, CiteType::WEBPAGE), url{url} {}

	void search() {
		httplib::Client client{ API_ENDPOINT };
		auto result = client.Get("/title/" + encodeUriComponent(url));
		if (result && result->status == 200) {
			nlohmann::json content = nlohmann::json::parse(result->body);
			webTitle = content["title"];
		}
		else {
			std::exit(1);
		}
	}

	void print(std::ostream& os) const {
		os << '[' << id << "] " << "webpage: " << webTitle 
			<< ". Available at " << url << std::endl;
	}
};

class Article : public Citation {
public:
	std::string title;
	std::string author;
	std::string journal;
	int year;
	int volume;
	int issue;

	Article(const std::string& id, const std::string& title, const std::string& author, const std::string& journal, 
		int year, int volume, int issue):
		Citation(id, CiteType::ARTICLE), title{ title }, author{author}, journal{ journal }, year{year}, volume{volume}, issue{issue}{ }

	void print(std::ostream& os) const {
		os << '[' << id << "] " << "article: " << author << ", " << title
			<< ", " << journal << ", " << year << ", " << volume << ", " << issue << std::endl;
	}
};
#endif