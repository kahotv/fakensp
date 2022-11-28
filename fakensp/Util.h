#pragma once
/* Tools */
class Util
{
public:
	/* Network */
	// 从堆栈里扫描上层的NSQUERY参数，可指定扫描深度
	static NSQUERY* SpiScanNsQuery(DWORD begin, DWORD deep);
	// 生成HOSTENT的BLOB形式
	static BLOB* PackHostEntBlob(IN const std::string& hostname, IN DWORD family, IN const sockaddr* addrlist, IN DWORD addrnum);

	/* System */ 
	// 取模块完整路径（包含文件名）
	static std::wstring GetModulePath(HMODULE h);
	// 取自身EXE完整路径（包含文件名）
	static std::wstring GetCurrentExePath();
	// Path转Dir
	static std::wstring Path2Dir(const std::wstring& path);
	
	// 取自身EXE目录，末尾带/（不包含文件名）
	static std::wstring GetCurrentExeDir();


};

/* String Tools */
class UtilString
{
public:
	static std::string TrimSpace(IN const std::string& str);				//移除首尾空格
	static std::wstring TrimSpace(IN const std::wstring& str);				//移除首尾空格
	static std::string ToLower(IN const std::string& str);					//转换到小写
	static std::wstring ToLower(IN const std::wstring& str);				//转换到小写
	static bool StartWith(IN const std::string& str, IN const std::string& val);	//字符串判断
	static bool StartWith(IN const std::wstring& str, IN const std::wstring& val);	//字符串判断
	static bool EndWith(IN const std::string& str, IN const std::string& val);		//判断是否以str2结束
	static bool EndWith(IN const std::wstring& str, IN const std::wstring& val);	//判断是否以str2结束
	static std::string w2s(const std::wstring &wstr);						//宽字符串->窄字符串
	static std::wstring s2w(const std::string &str);						//窄字符串->宽字符串
};

