#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <string>
# include <vector>
# include <iostream>

using namespace std;

class LocationConfig {
private:
	string			_path;
	string			_root;
	vector<string>	_index;
	bool			_autoindex;
	vector<string>	_allowedMethods;
	unsigned long	_maxBodySize;

	int				_returnCode;
	string			_returnUrl;

	string			_cgiExtension;
	string			_cgiPath;

	bool			_allowUpload;
	string			_uploadStore;

public:
	LocationConfig();
	~LocationConfig();

	const string&			getPath() const;
	const string&			getRoot() const;
	const vector<string>&	getIndex() const;
	bool					getAutoindex() const;
	const vector<string>&	getAllowedMethods() const;
	int						getReturnCode() const;
	const string&			getReturnUrl() const;
	const string&			getCgiExtension() const;
	const string&			getCgiPath() const;
	bool					getAllowUpload() const;
	const string&			getUploadStore() const;
	unsigned long			getMaxBodySize() const;

	void					setRoot(const string& root);
	void					setPath(const string& path);
	void					setIndex(const string& index);
	void					setAutoindex(bool autoindex);
	void					setAllowedMethods(const vector<string>& methods);
	void					setReturnCode(int code);
	void					setReturnUrl(const string& url);
	void					setCgiExtension(const string& ext);
	void					setCgiPath(const string& path);
	void					setAllowUpload(bool allow);
	void					setUploadStore(const string& store);
	void					setMaxBodySize(unsigned long size);
};

ostream& operator<<(ostream& os, const LocationConfig& loc);

#endif