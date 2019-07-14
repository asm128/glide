#include "glide.h"

#include "gpk_stdstring.h"
#include "gpk_process.h"
#include "gpk_storage.h"

::gpk::error_t									glide::validateMethod					(const ::gpk::view_const_string & method)	{
	::gpk::array_pod<char_t>							environmentBlock; 
	::gpk::array_obj<::gpk::TKeyValConstString>			environViews;
	::gpk::environmentBlockFromEnviron(environmentBlock);
	::gpk::environmentBlockViews(environmentBlock, environViews);
	for(uint32_t iKey = 0; iKey < environViews.size(); ++iKey) {
		if(environViews[iKey].Key == ::gpk::view_const_string{"REQUEST_METHOD"}) {
			if(environViews[iKey].Val == method) {
				return 1;
			}
		}
	}
	return 0;
}

::gpk::error_t									glide::loadCWD							(::gpk::array_pod<char_t> & cwd)	{
	::gpk::array_pod<char_t>							environmentBlock; 
	::gpk::array_obj<::gpk::TKeyValConstString>			environViews;
	::gpk::environmentBlockFromEnviron(environmentBlock);
	::gpk::environmentBlockViews(environmentBlock, environViews);
	for(uint32_t iKey = 0; iKey < environViews.size(); ++iKey) {
		if(environViews[iKey].Key == ::gpk::view_const_string{"SCRIPT_FILENAME"}) {
			cwd = environViews[iKey].Val;
			int32_t lastBarIndex = ::gpk::findLastSlash({cwd.begin(), cwd.size()});
			if(-1 != lastBarIndex) {
				cwd[lastBarIndex]		= 0;
				cwd.resize(lastBarIndex);
				break;
			}
		}
	}
	//"" : "C:/asm128_test/x64.Debug/test_cgi_environ.exe"
	return 0;
}

::gpk::error_t									glide::loadDetail						(int32_t & detail)	{
	::gpk::array_pod<char_t>							environmentBlock; 
	::gpk::array_obj<::gpk::TKeyValConstString>			environViews;
	::gpk::environmentBlockFromEnviron(environmentBlock);
	::gpk::environmentBlockViews(environmentBlock, environViews);
	for(uint32_t iKey = 0; iKey < environViews.size(); ++iKey) {
		if(environViews[iKey].Key == ::gpk::view_const_string{"PATH_INFO"}) {
			uint64_t _detail = (uint64_t)-1LL;
			::gpk::stoull({&environViews[iKey].Val[1], environViews[iKey].Val.size() - 1}, &_detail);
			detail = (int32_t)_detail;
		}
	}
	return 0;
}

static	const ::gpk::TKeyValConstString			g_DataBases	[]							=	// pair of database name/alias
	{	{"offices"		, "office"			}
	,	{"employees"	, "employee"		}	
	,	{"departments"	, "superdepartment"	}
	};

static	::gpk::error_t								createChildProcess		
	(	::glide::SProcess				& process
	,	::gpk::view_array<char_t>		environmentBlock
	,	::gpk::view_char				appPath
	,	::gpk::view_char				commandLine
	) {	// Create a child process that uses the previously created pipes for STDIN and STDOUT.
	bool													bSuccess				= false; 
	static constexpr const bool								isUnicodeEnv			= false;
	static constexpr const uint32_t							creationFlags			= (isUnicodeEnv ? CREATE_UNICODE_ENVIRONMENT : 0);
	gpk_safe_closehandle(process.ProcessInfo.hProcess);
	bSuccess											= CreateProcessA
		( appPath.begin()	// Create the child process. 
		, commandLine.begin()		// command line 
		, nullptr					// process security attributes 
		, nullptr					// primary thread security attributes 
		, true						// handles are inherited 
		, creationFlags				// creation flags 
		, environmentBlock.begin()	// use parent's environment 
		, NULL						// use parent's current directory 
		, &process.StartInfo		// STARTUPINFO pointer 
		, &process.ProcessInfo
		) ? true : false;  // receives PROCESS_INFORMATION 
	ree_if(false == bSuccess, "Failed to create process: '%s'.", appPath.begin());
	info_printf("Creating process '%s' with command line '%s'", appPath.begin(), commandLine.begin());
	return 0;
}

::gpk::error_t									glide::loadDatabase						(::glide::SGlideApp & appState)		{
	::gpk::array_obj<::glide::TKeyValDB>				& dbs									= appState.Databases;
	::glide::SQuery										& query									= appState.Query;
	query.Expand;
	dbs.resize(::gpk::size(g_DataBases));
	::gpk::array_pod<char_t>							szCmdlineApp							= "C:\\Python37\\python.exe";
	::gpk::array_pod<char_t>							szCmdlineFinal							= ::gpk::view_const_string{"C:\\Python37\\python.exe -c \"import requests; host = \"\"https://rfy56yfcwk.execute-api.us-west-1.amazonaws.com/bigcorp/employees\"\"; r = requests.get(host, \"\"\"\"); f = open(\"\""};
	szCmdlineFinal.append(appState.CWD);
	szCmdlineFinal.append(::gpk::view_const_string{"/employees.json\"\", \"\"w\"\"); f.write(str(r.text)); f.close();\""});
	{	// llamar proceso
		appState.ThreadState.Process.ProcessInfo.hProcess	= INVALID_HANDLE_VALUE;
		::gpk::array_pod<char_t>							environmentBlock; 
		::gpk::environmentBlockFromEnviron(environmentBlock);
		gerror_if(errored(::createChildProcess(appState.ThreadState.Process, environmentBlock, szCmdlineApp, szCmdlineFinal)), "Failed to create child process: %s.", szCmdlineApp.begin());	// Create the child process. 
		while(true) {
			DWORD												exitCode				= 0;
			GetExitCodeProcess(appState.ThreadState.Process.ProcessInfo.hProcess, &exitCode);
			if(STILL_ACTIVE != exitCode)
				break; 
			::gpk::sleep(5);
		}
	}
	info_printf("");
	for(uint32_t iDatabase = 0; iDatabase < ::gpk::size(g_DataBases); ++iDatabase) { 
		dbs[iDatabase].Key								= g_DataBases[iDatabase].Key;
		::gpk::array_pod<char_t>							filename								= g_DataBases[iDatabase].Key;
		filename.append(".json");
		gpk_necall(::gpk::jsonFileRead(dbs[iDatabase].Val, {filename.begin(), filename.size()}), "Failed to load json database file: %s.", filename.begin());
	}
	return 0;
}

::gpk::error_t									glide::loadQuery						(::glide::SQuery& query, const ::gpk::view_array<const ::gpk::TKeyValConstString> keyvals)	{
	::gpk::keyvalNumeric("offset"	, keyvals, query.Range.Offset	);
	::gpk::keyvalNumeric("limit"	, keyvals, query.Range.Count	);
	if(query.Range.Offset > 0 )
		--query.Range.Offset;
	{
		::gpk::error_t										indexExpand								= ::gpk::find("expand", keyvals);
		if(-1 != indexExpand) 
			query.Expand									= keyvals[indexExpand].Val;
	}
	return 0;
}

static	::gpk::error_t							generate_record_with_expansion			(::gpk::view_array<::glide::TKeyValDB> & databases, ::gpk::SJSONFile & database, uint32_t iRecord, ::gpk::array_pod<char_t> & output, const ::gpk::view_array<const ::gpk::view_const_char> & fieldsToExpand)	{
	::gpk::SJSONNode									& node									= *database.Reader.Tree[iRecord];
	if(0 == fieldsToExpand.size() || ::gpk::JSON_TYPE_OBJECT != node.Object->Type)
		::gpk::jsonWrite(&node, database.Reader.View, output);
	else {
		output.push_back('{');
		for(uint32_t iChild = 0; iChild < node.Children.size(); iChild += 2) { 
			uint32_t											indexKey								= node.Children[iChild + 0]->ObjectIndex;
			uint32_t											indexVal								= node.Children[iChild + 1]->ObjectIndex;
			::gpk::view_const_char								fieldToExpand							= fieldsToExpand[0];
			if(database.Reader.View[indexKey] == fieldToExpand && ::gpk::JSON_TYPE_NULL != database.Reader.Tree[indexVal]->Object->Type) {
				::gpk::jsonWrite(database.Reader.Tree[indexKey], database.Reader.View, output);
				output.push_back(':');
				bool												bSolved									= false;
				uint64_t											indexRecordToExpand						= 0;
				::gpk::stoull(database.Reader.View[indexVal], &indexRecordToExpand);
				--indexRecordToExpand;
				for(uint32_t iDatabase = 0; iDatabase < databases.size(); ++iDatabase) {
					::glide::TKeyValDB									& childDatabase							= databases[iDatabase];
					if(::gpk::view_const_char{childDatabase.Key.begin(), childDatabase.Key.size()-1} == fieldToExpand || g_DataBases[iDatabase].Val == fieldToExpand) {
						if(1 >= fieldsToExpand.size()) {
							if(indexRecordToExpand < childDatabase.Val.Reader.Tree[0]->Children.size())
								::gpk::jsonWrite(childDatabase.Val.Reader.Tree[0]->Children[(uint32_t)indexRecordToExpand], childDatabase.Val.Reader.View, output);
							else
								::gpk::jsonWrite(database.Reader.Tree[indexVal], database.Reader.View, output);
						}
						else {
							if(indexRecordToExpand < childDatabase.Val.Reader.Tree[0]->Children.size())
								::generate_record_with_expansion(databases, childDatabase.Val, childDatabase.Val.Reader.Tree[0]->Children[(uint32_t)indexRecordToExpand]->ObjectIndex, output, {&fieldsToExpand[1], fieldsToExpand.size()-1});
							else
								::gpk::jsonWrite(database.Reader.Tree[indexVal], database.Reader.View, output);
						}
						bSolved											= true;
					}
				}
				if(false == bSolved) 
					::gpk::jsonWrite(database.Reader.Tree[indexVal], database.Reader.View, output);
			}
			else {
				::gpk::jsonWrite(database.Reader.Tree[indexKey], database.Reader.View, output);
				output.push_back(':');
				::gpk::jsonWrite(database.Reader.Tree[indexVal], database.Reader.View, output);
			}
			if((node.Children.size() - 2) > iChild)
				output.push_back(',');
		}
		output.push_back('}');
	}
	return 0;
}

::gpk::error_t									glide::generate_output_for_db			(::glide::SGlideApp & app, const ::gpk::view_const_string & databaseName, int32_t detail, ::gpk::array_pod<char_t> & output)					{
	int32_t												indexDB									= ::gpk::find(databaseName, ::gpk::view_array<const ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile>>{app.Databases.begin(), app.Databases.size()});
	::gpk::SJSONReader									& dbReader								= app.Databases[indexDB].Val.Reader;
	::gpk::SJSONNode									& jsonRoot								= *app.Databases[indexDB].Val.Reader.Tree[0];
	if(detail != -1) { // display detail
		if(detail > 0) 
			--detail;
		if(0 == app.Query.Expand.size() && ((uint32_t)detail) >= jsonRoot.Children.size())
			::gpk::jsonWrite(&jsonRoot, dbReader.View, output);
		else {
			if(0 == app.Query.Expand.size()) {
				::gpk::jsonWrite(jsonRoot.Children[detail], dbReader.View, output);
			}
			else {
				::gpk::array_obj<::gpk::view_const_char>			fieldsToExpand;
				::gpk::split(app.Query.Expand, '.', fieldsToExpand);
				::generate_record_with_expansion(app.Databases, app.Databases[indexDB].Val, jsonRoot.Children[detail]->ObjectIndex, output, fieldsToExpand);
			}
		}
	}
	else {  // display multiple records
		if(0 == app.Query.Expand.size() && 0 >= app.Query.Range.Offset && app.Query.Range.Count >= jsonRoot.Children.size())
			::gpk::jsonWrite(&jsonRoot, dbReader.View, output);
		else {
			output.push_back('[');
			const uint32_t										stopRecord								= ::gpk::min(app.Query.Range.Offset + app.Query.Range.Count, jsonRoot.Children.size());
			if(0 == app.Query.Expand.size()) {
				for(uint32_t iRecord = app.Query.Range.Offset; iRecord < stopRecord; ++iRecord) {
					::gpk::jsonWrite(jsonRoot.Children[iRecord], dbReader.View, output);
					if((stopRecord - 1) > iRecord)
						output.push_back(',');
				}
			}
			else {
				::gpk::array_obj<::gpk::view_const_char>			fieldsToExpand;
				::gpk::split(app.Query.Expand, '.', fieldsToExpand);
				for(uint32_t iRecord = app.Query.Range.Offset; iRecord < stopRecord; ++iRecord) {
					::generate_record_with_expansion(app.Databases, app.Databases[indexDB].Val, jsonRoot.Children[iRecord]->ObjectIndex, output, fieldsToExpand);
					if((stopRecord - 1) > iRecord)
						output.push_back(',');
				}
			}
			output.push_back(']');
		}
	}
	return 0;
}
