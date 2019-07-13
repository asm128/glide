#include "glide.h"

#include "gpk_cgi_app_impl.h"

#ifndef GLIDE_ENDPOINT_H_20190713
#define GLIDE_ENDPOINT_H_20190713

#define GLIDE_ENDPOINT_IMPL(_endpointName)																																			\
GPK_CGI_JSON_APP_IMPL();																																							\
																																													\
::gpk::error_t									generate_output					(::gpk::SCGIRuntimeValues & runtimeValues, ::gpk::array_pod<char_t> & output)					{	\
	output.append(::gpk::view_const_string{"\r\n"});																																\
	::glide::SGlideApp									app;																														\
	gpk_necall(::glide::databaseLoad(app.Databases), "%s", "Failed to load glide databases.");																						\
	gpk_necall(::glide::queryLoad(app.Query, runtimeValues.QueryStringKeyVals), "%s", "Failed to load glide databases.");															\
	gpk_necall(::glide::generate_output_for_db(app, _endpointName, output), "%s", "Failed to load glide databases.");																\
	return 0;																																										\
}

#endif // GLIDE_ENDPOINT_H_20190713
