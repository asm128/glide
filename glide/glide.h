#include "gpk_json.h"
#include "gpk_keyval.h"

#include <Windows.h>
#include <process.h>

#ifndef GLIDE_H_20190712
#define GLIDE_H_20190712

namespace glide
{
	typedef ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile> TKeyValDB;

	static constexpr const uint32_t					MAX_TABLE_RECORD_COUNT		= 0x7FFFFFFF;

	struct SQuery {
		::gpk::SRange<uint32_t>						Range						= {0, MAX_TABLE_RECORD_COUNT};
		::gpk::view_const_string					Expand						= "";
	};


	struct SProcess {
		PROCESS_INFORMATION							ProcessInfo					= {}; 
		STARTUPINFOA								StartInfo					= {sizeof(STARTUPINFOA)};
	};

	struct SGlideApp {
		::gpk::array_obj<TKeyValDB>					Databases					= {};
		::glide::SQuery								Query						= {};
		::glide::SProcess							Process				;
		::gpk::array_pod<char_t>					CWD							= {};
	};

	::gpk::error_t									validateMethod				(const ::gpk::view_const_string & method);
	::gpk::error_t									loadCWD						(::gpk::array_pod<char_t> & method);
	::gpk::error_t									loadDetail					(int32_t & detail);
	::gpk::error_t									loadQuery					(::glide::SQuery& query, const ::gpk::view_array<const ::gpk::TKeyValConstString> keyvals);
	::gpk::error_t									loadDatabase				(::glide::SGlideApp & appState);

	typedef ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile> TKeyValDB;

	::gpk::error_t									generate_output_for_db		(::glide::SGlideApp & app, const ::gpk::view_const_string & databaseName, int32_t detail, ::gpk::array_pod<char_t> & output);
}


#endif // GLIDE_H_20190712
