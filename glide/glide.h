#include "gpk_json.h"
#include "gpk_keyval.h"

#ifndef GLIDE_H_20190712
#define GLIDE_H_20190712

namespace glide
{
	typedef ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile> TKeyValDB;

	::gpk::error_t									validateMethod				(const ::gpk::view_const_string & method);
	::gpk::error_t									databaseLoad				(::gpk::array_obj<::glide::TKeyValDB> & dbs);

	static constexpr const uint32_t					MAX_TABLE_RECORD_COUNT		= 0x7FFFFFFF;

	struct SQuery {
		::gpk::SRange<uint32_t>							Range						= {0, MAX_TABLE_RECORD_COUNT};
		::gpk::view_const_string						Expand						= "";
	};

	::gpk::error_t									queryLoad					(::glide::SQuery& query, const ::gpk::view_array<const ::gpk::TKeyValConstString> keyvals);

	typedef ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile> TKeyValDB;

	struct SGlideApp {
		::gpk::array_obj<TKeyValDB>						Databases					= {};
		::glide::SQuery									Query						= {};
	};

	::gpk::error_t									generate_output_for_db		(::glide::SGlideApp & app, const ::gpk::view_const_string & databaseName, ::gpk::array_pod<char_t> & output);
}


#endif // GLIDE_H_20190712
