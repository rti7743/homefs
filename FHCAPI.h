#pragma once

#include <string>
#include <vector>

struct Action
{
	std::string name;
	
};

struct Elec
{
	std::string name;
	std::string status;

	std::vector<Action*> action;
};


class FHCAPI
{
public:
	FHCAPI()
	{
	}
	virtual ~FHCAPI()
	{
		freeElec();
	}

	//起動パラメタのパース
	bool setInit(const std::string& mountpoint,const std::string& key);

	//状態の更新
	bool update();
	
	const std::vector<Elec*>* getElec() const
	{
		return &this->elec;
	}

	const Elec* findElec(const std::string& name) const;

	//この状態名があるかどうか調べようか.
	bool checkStatus(const std::string& elecname,const std::string& actionname) const;
	
	//家電の名前一覧を返します
	void api_getlist(std::vector<std::string>* outVec);
	//家電のステータスを返します
	void api_getinfo(const std::string& name,std::map<std::string,std::string>* outMap);
	//家電の状態一覧を返します
	void api_getactionlist(const std::string& name,std::vector<std::string>* outVec);
	//家電をその状態にする.
	bool fire(const std::string& elecname,const std::string& actionname) ;
private:
	std::string host;
	std::string apikey;

	std::vector<Elec*> elec;


	void parseJsonList(const std::string json,std::vector<std::string>* outVec) const;
	void parseJsonToMap(const std::string json,std::map<std::string,std::string>* outMap) const;
	void freeElec();
};

