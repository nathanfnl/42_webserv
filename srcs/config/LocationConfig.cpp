#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig()
: _autoindex(false),
    _maxBodySize(0),
    _returnCode(0),
    _allowUpload(false) {}

LocationConfig::~LocationConfig() {}

const string& LocationConfig::getPath() const {
    return this->_path;
}

const string& LocationConfig::getRoot() const {
    return this->_root;
}

const vector<string>& LocationConfig::getIndex() const {
    return this->_index;
}

bool LocationConfig::getAutoindex() const {
    return this->_autoindex;
}

const vector<string>& LocationConfig::getAllowedMethods() const {
    return this->_allowedMethods;
}

int LocationConfig::getReturnCode() const {
    return this->_returnCode;
}

const string& LocationConfig::getReturnUrl() const {
    return this->_returnUrl;
}

const string& LocationConfig::getCgiExtension() const {
    return this->_cgiExtension;
}

const string& LocationConfig::getCgiPath() const {
    return this->_cgiPath;
}

bool LocationConfig::getAllowUpload() const {
    return this->_allowUpload;
}

const string& LocationConfig::getUploadStore() const {
    return this->_uploadStore;
}

unsigned long LocationConfig::getMaxBodySize() const {
    return this->_maxBodySize;
}

void LocationConfig::setPath(const string& path) {
    this->_path = path;
}

void LocationConfig::setRoot(const string& root) {
    this->_root = root;
}

void LocationConfig::setIndex(const string& index) {
    this->_index.push_back(index);
}

void LocationConfig::setAutoindex(bool autoindex) {
    this->_autoindex = autoindex;
}

void LocationConfig::setAllowedMethods(const vector<string>& methods) {
    this->_allowedMethods = methods;
}

void LocationConfig::setReturnCode(int code) {
    this->_returnCode = code;
}

void LocationConfig::setReturnUrl(const string& url) {
    this->_returnUrl = url;
}

void LocationConfig::setCgiExtension(const string& ext) {
    this->_cgiExtension = ext;
}

void LocationConfig::setCgiPath(const string& path) {
    this->_cgiPath = path;
}

void LocationConfig::setAllowUpload(bool allow) {
    this->_allowUpload = allow;
}

void LocationConfig::setUploadStore(const string& store) {
    this->_uploadStore = store;
}

void LocationConfig::setMaxBodySize(unsigned long size) {
    this->_maxBodySize = size;
}

ostream& operator<<(ostream& os, const LocationConfig& loc)
{
    os << "  Location: [" << loc.getPath() << "]\n";
    os << "    Root            : " << (loc.getRoot().empty() ? "(none)" : loc.getRoot()) << "\n";

    os << "    Index           : ";
    if (loc.getIndex().empty()) {
        os << "(none)";
    } else {
        for (size_t i = 0; i < loc.getIndex().size(); ++i) {
            os << loc.getIndex()[i] << (i < loc.getIndex().size() - 1 ? " " : "");
        }
    }
    os << "\n";

    os << "    Autoindex       : " << (loc.getAutoindex() ? "on" : "off") << "\n";

    os << "    Allowed Methods : ";
    if (loc.getAllowedMethods().empty()) {
        os << "(all)";
    } else {
        for (size_t i = 0; i < loc.getAllowedMethods().size(); ++i) {
            os << loc.getAllowedMethods()[i] << (i < loc.getAllowedMethods().size() - 1 ? " " : "");
        }
    }
    os << "\n";

    os << "    Max Body Size   : " << loc.getMaxBodySize() << " bytes\n";

    if (loc.getReturnCode() != 0) {
        os << "    Return          : " << loc.getReturnCode() << " " << loc.getReturnUrl() << "\n";
    }

    if (!loc.getCgiExtension().empty()) {
        os << "    CGI             : " << loc.getCgiExtension() << " -> " << loc.getCgiPath() << "\n";
    }

    os << "    Upload          : " << (loc.getAllowUpload() ? "on" : "off");
    if (loc.getAllowUpload() && !loc.getUploadStore().empty()) {
        os << " (Store: " << loc.getUploadStore() << ")";
    }
    os << "\n";

    return os;
}
