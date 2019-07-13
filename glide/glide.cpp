#include "glide.h"

#include "gpk_stdstring.h"

static	const ::gpk::TKeyValConstString			g_DataBases	[]					=	// pair of name/alias
	{	{"offices"		, "office"			}
	,	{"employees"	, "employee"		}	
	,	{"departments"	, "superdepartment"	}
	};

::gpk::error_t									glide::databaseLoad						(::gpk::array_obj<::glide::TKeyValDB> & dbs)		{
	dbs.resize(::gpk::size(g_DataBases));
	for(uint32_t iDatabase = 0; iDatabase < ::gpk::size(g_DataBases); ++iDatabase) { 
		dbs[iDatabase].Key								= g_DataBases[iDatabase].Key;
		::gpk::array_pod<char_t>							filename								= g_DataBases[iDatabase].Key;
		filename.append(".json");
		gpk_necall(::gpk::jsonFileRead(dbs[iDatabase].Val, {filename.begin(), filename.size()}), "Failed to load json database file: %s.", filename.begin());
	}
	return 0;
}

::gpk::error_t									glide::queryLoad						(::glide::SQuery& query, const ::gpk::view_array<const ::gpk::TKeyValConstString> keyvals)	{
	::gpk::keyvalNumeric("offset"	, keyvals, query.Range.Offset	);
	::gpk::keyvalNumeric("limit"	, keyvals, query.Range.Count	);
	{
		::gpk::error_t										indexExpand								= ::gpk::find("expand", keyvals);
		if(-1 != indexExpand) 
			query.Expand									= keyvals[indexExpand].Val;
	}
	return 0;
}

static	::gpk::error_t							generate_record_with_expansion			(::gpk::view_array<::glide::TKeyValDB> & databases, ::gpk::SJSONFile & database, uint32_t iRecord, ::gpk::array_pod<char_t> & output, const ::gpk::view_array<::gpk::view_const_char> & fieldsToExpand)	{
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
							output.append(::gpk::view_const_string{"\"Insert next "});
							output.append(database.Reader.View[indexKey]);
							output.append(::gpk::view_const_string{" "});
							output.append(database.Reader.View[indexVal]);
							output.append(::gpk::view_const_string{" query result here\""});
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

::gpk::error_t									glide::generate_output_for_db			(::glide::SGlideApp & app, const ::gpk::view_const_string & databaseName, ::gpk::array_pod<char_t> & output)					{
	int32_t												indexDB									= ::gpk::find(databaseName, ::gpk::view_array<const ::gpk::SKeyVal<::gpk::view_const_string, ::gpk::SJSONFile>>{app.Databases.begin(), app.Databases.size()});
	::gpk::SJSONReader									& dbReader								= app.Databases[indexDB].Val.Reader;
	::gpk::SJSONNode									& jsonRoot								= *app.Databases[indexDB].Val.Reader.Tree[0];
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
	return 0;
}
