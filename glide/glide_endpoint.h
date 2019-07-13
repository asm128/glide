#include "glide.h"

#include "gpk_cgi_app_impl.h"

#include "gpk_process.h"

#ifndef GLIDE_ENDPOINT_H_20190713
#define GLIDE_ENDPOINT_H_20190713

#define GLIDE_READONLY_ENDPOINT_IMPL(_endpointName)																																	\
GPK_CGI_JSON_APP_IMPL();																																							\
																																													\
::gpk::error_t									generate_output					(::gpk::SCGIRuntimeValues & runtimeValues, ::gpk::array_pod<char_t> & output)					{	\
	output.append(::gpk::view_const_string{"\r\n"});																																\
	::glide::SGlideApp									app;																														\
	::gpk::array_obj<::gpk::TKeyValConstString>			environViews;																												\
	::gpk::environmentBlockViews(runtimeValues.EntryPointArgs.EnvironmentBlock, environViews);																						\
	if(0 == ::glide::validateMethod("GET")) {																																		\
		output.append(::gpk::view_const_string{"{ \"status\" : 403, \"description\" :\"forbidden\" }\r\n"});																		\
		return 1;																																									\
	}																																												\
	gpk_necall(::glide::loadDatabase(app.Databases), "%s", "Failed to load glide databases.");																						\
	gpk_necall(::glide::loadQuery(app.Query, runtimeValues.QueryStringKeyVals), "%s", "Failed to load query.");																		\
	int32_t												detail							= -1;																						\
	gpk_necall(::glide::loadDetail(detail), "%s", "Failed to load detail.");																										\
	gpk_necall(::glide::generate_output_for_db(app, _endpointName, detail, output), "%s", "Failed to load glide databases.");														\
	return 0;																																										\
}

#endif // GLIDE_ENDPOINT_H_20190713
