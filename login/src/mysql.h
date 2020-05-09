#pragma once

#ifndef __mysqlq1_h__
#define __mysqlq1_h__

#include <winsock2.h>
#include <mysql.h>
#include <map>
#include <vector>
#include <string>
#include "error.h"
#include <stdexcept>

#include "logger.h"

void reconnectsql(MYSQL **con);

class sqlquery
{
	std::string str1, ustr, table;
	MYSQL_FIELD *field;
    MYSQL_RES *res;
    MYSQL_ROW row;
	MYSQL **con;
	std::map<std::string, int> fieldnames;
	std::map<std::string, int> tablefieldnames;
	bool ufirst;
	int nc;
	void query()
	{
		int a,b=0;
		freeup();
tryagain:
		if(mysql_query(*con, str1.c_str()))
		{
			a=mysql_errno(*con);
			if(((a==2006)||(a==2013))&&(b<5))	//server gone away
			{
				if(a==2006)logger.log("error %d server gone away. try %d\n",a, b);
				else logger.log("error %d Lost connection to MySQL server during query. try %d\n",a, b);
				reconnectsql(con);
				b++;
				goto tryagain;
			}
			clearupdate();
			freeup();
			std::string errorstring="Mysql query error: ";
			errorstring+=::mysql_error(*con);
			errorstring+=" query: ";
			errorstring+=str1;
			throw error(errorstring, "mysql query");
		}
		if(mysql_field_count(*con)>0)
		{
			res=mysql_use_result(*con);
			nc=mysql_num_fields(res);
			a=0;
			fieldnames.clear();
			while((field = mysql_fetch_field(res)))
			{
				fieldnames.insert(std::pair<std::string, int>(field->name, a));
				a++;
			}

		}else nc=-1;
	}
public:
	sqlquery(MYSQL **c, const std::string &table1)
		:nc(0),table(table1),con(c),ufirst(true),res(0)
	{
		int a=0;
		str1="describe "+table;
		query();
		while(next())
		{
			tablefieldnames.insert(std::pair<std::string, int>(row[0], a));
			a++;
		}
		freeup();
		str1.reserve(4096);
		clearupdate();
	}
	void init(MYSQL **c, const std::string &table1)
	{
		nc=0;
		table=table1;
		con=c;
		ufirst=true;

		int a=0;
		str1="describe "+table;
		query();
		while(next())
		{
			tablefieldnames.insert(std::pair<std::string, int>(row[0], a));
			a++;
		}
		freeup();
		str1.reserve(4096);
		clearupdate();
	}
	sqlquery():res(0){}
	~sqlquery()
	{
		freeup();
	}
	bool next()
	{
		row = mysql_fetch_row(res);
		if(row==0)freeup();
		return (row!=0);
	}

	int affectedRows()
	{
		return (int)mysql_affected_rows(*con);
	}

	void select()
	{
		str1.clear();
		str1.reserve(4096);
		str1+="SELECT * FROM ";
		str1+=table;
		str1+=';';
		query();
	}
	void select(const std::string &mit)
	{
		str1.clear();
		str1.reserve(4096);
		str1+="SELECT ";
		str1+=mit;
		str1+=" FROM ";
		str1+=table;
		str1+=';';
		query();
	}
	void selectw(const std::string &opt)
	{
		str1.clear();
		str1.reserve(4096);
		str1+="SELECT * FROM ";
		str1+=table;
		str1+=" WHERE ";
		str1+=opt;
		str1+=';';
		query();
	}

	void selectw(const std::string &opt, const std::string &mit)
	{
		str1.clear();
		str1.reserve(4096);
		str1+="SELECT ";
		str1+=mit;
		str1+=" FROM ";
		str1+=table;
		str1+=" WHERE ";
		str1+=opt;
		str1+=';';
		query();
	}
	void del(const std::string &opt)
	{
		str1.clear();
		str1.reserve(4096);
		str1+="DELETE FROM ";
		str1+=table;
		str1+=" WHERE ";
		str1+=opt;
		str1+=';';
		query();
	}
	void insert()
	{
		str1.clear();
		str1.reserve(4096);
		str1+="INSERT INTO ";
		str1+=table;
		str1+=" SET ";
		str1+=ustr;
		str1+=';';
		query();
		clearupdate();
	}
	void replace()
	{
		str1.clear();
		str1.reserve(4096);
		str1+="REPLACE LOW_PRIORITY INTO ";
		str1+=table;
		str1+=" SET ";
		str1+=ustr;
		str1+=';';
		query();
		clearupdate();
	}
	void update(const std::string &opt)
	{
		str1.clear();
		str1.reserve(4096);
		str1+="UPDATE ";
		str1+=table;
		str1+=" SET ";
		str1+=ustr;
		str1+=" WHERE ";
		str1+=opt;
		str1+=';';
		query();
		clearupdate();
	}

	void clearupdate()
	{
		ustr.clear();
		ustr.reserve(2048);
		ufirst=true;
	}

	int getColumnIndex(const std::string &s)
	{
		int a=-1;
		std::map<std::string, int>::iterator i;
		i=this->fieldnames.find(s);
		if(i!=this->fieldnames.end())a=i->second;
		return a;
	}
	int getTableColumnIndex(const std::string &s)
	{
		int a=-1;
		std::map<std::string, int>::iterator i;
		i=this->tablefieldnames.find(s);
		if(i!=this->tablefieldnames.end())a=i->second;
		return a;
	}

	void addupdate(const std::string &mit, const std::string &mire)
	{
		if(!ufirst)ustr+=',';
		ustr+=mit;
		ustr+="='";
		ustr+=mire;
		ustr+="' ";
		ufirst=false;
	}

	void addnull(const std::string &mit)
	{
		if(!ufirst)ustr+=',';
		ustr+=mit;
		ustr+="=null ";
		ufirst=false;
	}

	void addeupdate(const std::string &mit, char *p, int lp)
	{
		char *q;//, *s;
		if(lp<=0)return;
//		s=new char[lp+1];
//		int a=0;
//		for(;a<lp-4;a+=4)*(unsigned int*)(&s[a])=~*(unsigned int*)(&p[a]);
//		for(;a<lp;a++)s[a]=~p[a];
		q=new char[lp*2+1];
		mysql_real_escape_string(*con, q, p, lp);
		if(!ufirst)ustr+=',';
		ustr+=mit;
		ustr+="='";
		ustr+=&q[0];
		ustr+="' ";
		delete[] q;

		ufirst=false;
	}

	void freeup()
	{
		if(res!=0)
		{
			mysql_free_result(res);
			res=0;
			nc=-1;
			this->fieldnames.clear();
		}
	}
	std::string operator[](int a)
	{
		std::string str;
		if((a<0)||(a>=nc))throw std::range_error("sql column range error");
		if(row[a]!=0)str=row[a];
		return str;
	}
	std::string operator[](const std::string &cn)
	{
		std::string str;
		int a=-1;
		std::map<std::string, int>::iterator i;
		i=this->fieldnames.find(cn);
		if(i!=this->fieldnames.end())a=i->second;
		if(a==-1)throw std::range_error("sql column range error: field doesnt exists");
		if(row[a]!=0)str=row[a];
		return str;
	}
	char* getptr(int a)
	{
		if((a<0)||(a>=nc))throw std::range_error("sql column range error");
		return row[a];
	}
	char* getptr(const std::string &cn)
	{
		int a=-1;
		std::map<std::string, int>::iterator i;
		i=this->fieldnames.find(cn);
		if(i!=this->fieldnames.end())a=i->second;
		if(a==-1)throw std::range_error("sql column range error: field doesnt exists");
		return row[a];
	}

};

#endif
