#pragma once

#ifndef __mysqlq1_h__
#define __mysqlq1_h__


#include <winsock2.h>
#include <mysql/mysql.h>
#include <map>
#include <set>
#include "vector2.h"
#include <string>
#include "error.h"

#include "logger.h"
#include "pmutex.h"


class mcon
{
	std::string host, user, passwd, db;
	pmutex mcmutex;
	MYSQL *con;
	void reconnect_l()
	{
		my_bool reconnect = 1;
		int a=0;
		for(a=0;a<1024;a++)
		{
			mysql_close(con);
			con=mysql_init(0);
			if(!mysql_real_connect(con, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 0, (char*)0, 0))
			{
				Sleep(1);
			}else 
			{
				//mysql_options(con, MYSQL_OPT_RECONNECT, &reconnect);
				break;
			}
		}
	}
	mcon(const mcon&);
	void operator=(const mcon&);
public:
	mcon():con(0){}
	~mcon()
	{
		if(con!=0)mysql_close(con);
		con=0;
	}
	void reconnect()
	{
		ul m=mcmutex.lock();
		reconnect_l();

	}
	bool init(char *mysqlhost, char *mysqluser, char *mysqlpasswd, char *mysqldb)
	{
		ul m=mcmutex.lock();
		my_bool reconnect = 1;

		host=mysqlhost;
		user=mysqluser;
		passwd=mysqlpasswd;
		db=mysqldb;
		con=mysql_init(0);
		if(!mysql_real_connect(con, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 0, (char*)0, 0))
		{
			return false;
		}

		//mysql_options(con, MYSQL_OPT_RECONNECT, &reconnect);

		return true;
	}
	bool query(const std::string &str, MYSQL_RES **res, int &fieldcount, int &errnum, std::string &errstr)
	{
		ul m=mcmutex.lock();
//		sqllogger.log("%s\n", str.c_str());
		mysql_ping(con);
		*res=0;
		fieldcount=0;
		bool r;
		r= (mysql_query(con, str.c_str())!=0);
		if(!r)
		{
			fieldcount=(int)mysql_field_count(con);
			if(fieldcount>0)*res=mysql_store_result(con);
		}else
		{
			errstr=mysql_error(con);
			errnum=mysql_errno(con);
		}
		return r;
	}

	void real_escape_string(char *q, char *p, int lp)
	{
		ul m=mcmutex.lock();
		mysql_real_escape_string(con, q, p, lp);
	}

};

//void reconnectsql(MYSQL **con);

class sqlquery
{
	std::string str1, ustr, table;
	MYSQL_FIELD *field;
    MYSQL_RES *res;
    MYSQL_ROW row;
//	MYSQL **con;
	mcon *con;
	std::map<std::string, int> fieldnames;
	std::map<std::string, int> tablefieldnames;
	std::set<std::string> columnguard;
	bool ufirst;
	int nc;
	void query()
	{
		int a,b=0;
		int fieldcount;
		std::string errorstring;
		freeup();
tryagain:
		if(con->query(str1.c_str(), &res, fieldcount, a, errorstring))
		{
//			a=con->errnum();
			if(((a==2006)||(a==2013))&&(b<128))	//server gone away
			{
				if(a==2006)logger.log("error %d server gone away. try %d\n",a, b);
				else logger.log("error %d Lost connection to MySQL server during query. try %d\n",a, b);
				con->reconnect();
//				reconnectsql(con);
				b++;
				goto tryagain;
			}
			clearupdate();
			freeup();
			errorstring="Mysql query error: "+errorstring;
			errorstring+=" query: ";
			errorstring+=str1;
			throw error(errorstring, "mysql query");
		}
		if(fieldcount>0)
		{
//			res=con->store_result();
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
	void query(const std::string &q)
	{
		str1=q;
		query();
	}
	sqlquery(mcon *c, const std::string &table1)
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
		str1="";
		str1.reserve(4096);
		clearupdate();
	}
	void init(mcon *c, const std::string &table1)
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
		str1="";
		str1.reserve(4096);
		clearupdate();
	}
	sqlquery():res(0),con(0),nc(0),table(""),ufirst(true){}
	sqlquery(mcon *c):nc(0),table(""),con(c),ufirst(true),res(0){}
	~sqlquery()
	{
		freeup();
	}
	void settable(const std::string &name, bool getcolumns=false)
	{
		table=name;
		if(getcolumns)
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
			str1="";
			str1.reserve(4096);
		}
	}
	bool next()
	{
		row = mysql_fetch_row(res);
		if(row==0)freeup();
		return (row!=0);
	}
/*
	int affectedRows()
	{
		return (int)con->affected_rows();
	}
*/
	void select()
	{
		str1.clear();
		str1.reserve(4096);
		str1="SELECT * FROM ";
		str1+=table;
		str1+=';';
		query();
	}
	void select(const std::string &mit)
	{
		str1.clear();
		str1.reserve(4096);
		str1="SELECT ";
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
		str1="SELECT * FROM ";
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
		str1="SELECT ";
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
		str1="DELETE FROM ";
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
		str1="INSERT INTO ";
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
		str1="REPLACE LOW_PRIORITY INTO ";
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
		str1="UPDATE ";
		str1+=table;
		str1+=" SET ";
		str1+=ustr;
		str1+=" WHERE ";
		str1+=opt;
		str1+=';';
		query();
		clearupdate();
	}

	void beginupdate()
	{
		clearupdate();
	}
	void clearupdate()
	{
		ustr="";
//		ustr.clear();
		ustr.reserve(2048);
		ufirst=true;
		columnguard.clear();
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
		checkcolumn(mit);
		if(!ufirst)ustr+=',';
		ustr+=mit;
		ustr+="='";
		ustr+=mire;
		ustr+="' ";
		ufirst=false;
	}

	void addnull(const std::string &mit)
	{
		checkcolumn(mit);
		if(!ufirst)ustr+=',';
		ustr+=mit;
		ustr+="=null ";
		ufirst=false;
	}

	void addeupdate(const std::string &mit, char *p, int lp)
	{
		checkcolumn(mit);
		char *q;//, *s;
		if(lp<=0)return;
//		s=new char[lp+1];
//		int a=0;
//		for(;a<lp-4;a+=4)*(unsigned int*)(&s[a])=~*(unsigned int*)(&p[a]);
//		for(;a<lp;a++)s[a]=~p[a];
		q=new char[lp*2+1];
		con->real_escape_string(q, p, lp);
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
		clearupdate();
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
	void checkcolumn(const std::string &s)
	{
/*		
		if(columnguard.empty())
		{
			columnguard.insert(s);
		}else
		{
			std::set<std::string>::iterator i=columnguard.find(s);
			if(i==columnguard.end())
			{
				columnguard.insert(s);
			}else
			{
				freeup();
				throw error("Multiple column " + s + " in query", "sqlquery::checkcolumn");
			}
		}
*/		
	}
};

#endif
