#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <map>
#include <stdarg.h>
#include <sys/stat.h>
#include "SCMLpp.h"
#include <math.h>
#include <modtoollib/modtool.h>

using namespace std;
using namespace Compat;

#define IN
#define OUT
#define MAX_NAME_LENGTH 256
#define FRAME_RATE 40


//#define ANIMDEBUG 1
#define NEWPIVOTS 1


typedef SCML::Data s_data;
typedef SCML::Data::Folder s_folder;
typedef SCML_MAP( int, s_folder* ) s_folder_map;
typedef SCML::Data::Folder::File s_file;
typedef SCML_MAP( int, s_file* ) s_file_map;
typedef SCML::Data s_data;
typedef SCML::Data::Entity::Animation s_animation;
typedef SCML_MAP( int, s_animation* ) s_animation_map;
typedef SCML::Data::Entity::Animation::Mainline s_mainline;
typedef SCML::Data::Entity::Animation::Mainline::Key s_mainline_key;
typedef SCML_MAP( int, s_mainline_key* ) s_mainline_key_map;
typedef SCML::Data::Entity::Animation::Mainline::Key::Bone_Ref s_bone_ref;
typedef SCML::Data::Entity::Animation::Mainline::Key::Bone_Container s_bone_container;
typedef SCML_MAP( int, s_bone_container ) s_bone_container_map;
typedef SCML::Data::Entity::Animation::Mainline::Key::Object_Container s_object_container;
typedef SCML_MAP( int, s_object_container ) s_object_container_map;
typedef SCML::Data::Entity::Animation::Mainline::Key::Object_Ref s_object_ref;
typedef SCML::Data::Entity s_entity;
typedef SCML_MAP( int, s_entity* ) s_entity_map;
typedef SCML::Data::Entity::Animation::Timeline s_timeline;
typedef SCML_MAP( int, s_timeline* ) s_timeline_map;
typedef SCML::Data::Entity::Animation::Timeline::Key s_timeline_key;
typedef SCML_MAP( int, s_timeline_key* ) s_timeline_key_map;
typedef SCML::Data::Entity::Animation::Timeline::Key::Object s_timeline_object;
typedef SCML::Data::Entity::Animation::Timeline::Key::Bone s_timeline_bone;

template<typename T>
struct PlanarPoint
{
    T x;
    T y;

	PlanarPoint() : x(0), y(0) {}
	PlanarPoint(T _x, T _y) : x(_x), y(_y) {}
	PlanarPoint(const PlanarPoint& p) : x(p.x), y(p.y) {}

	template<typename U>
	explicit PlanarPoint(const PlanarPoint<U>& p) : x(p.x), y(p.y) {}
};

typedef PlanarPoint<float> float2;
typedef PlanarPoint<int> int2;

struct rectangle {
	float x1, x2;
	float y1, y2;
};

struct bounding_box
{
	float x;
	float y;
	int w;
	int h;

private:
	void set_values(float _x, float _y, int _w, int _h) {
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}

public:
	bounding_box() {}
	bounding_box(float _x, float _y, int _w, int _h) {
		set_values(_x, _y, _w, _h);
	}
	bounding_box(const float2& pos, const int2& dim) {
		set_values(pos.x, pos.y, dim.x, dim.y);
	}
	explicit bounding_box(const rectangle& r) {
		set_values(r.x1, r.y1, int(ceil(r.x2 - r.x1)), int(ceil(r.y2 - r.y1)));
	}

	void split(float2& pos, int2& dim) const {
		pos.x = x;
		pos.y = y;
		dim.x = w;
		dim.y = h;
	}
};


typedef bounding_box symbol_frame_metadata_t;
typedef std::vector<symbol_frame_metadata_t> symbol_metadata_t;
typedef std::map<std::string, symbol_metadata_t> build_metadata_t;

struct matrix2
{
    float m00;
    float m01;
    float m10;
    float m11;
};

struct symbol_id
{
    int folder;
    int file;
};

struct matrix3
{
    matrix3()
    :   m00( 1.f )
    ,   m01( 0.f )
    ,   m02( 0.f )
    ,   m10( 0.f )
    ,   m11( 1.f )
    ,   m12( 0.f )
    ,   m20( 0.f )
    ,   m21( 0.f )
    ,   m22( 1.f )
    {

    }

    matrix3& set_translation( float x, float y )
    {
        m02 = x;
        m12 = y;
        return *this;
    }

	matrix3& set_translation( const float2& v )
	{
		return set_translation( v.x, v.y );
	}

	float2 get_translation() const {
		return float2(m02, m12);
	}

    matrix3& set_scale( float x, float y )
    {
        m00 = x;
        m11 = y;
        return *this;
    }

    matrix3& set_rotation( float angle )
    {
        m00 = cos( angle );
        m01 = sin( angle );
        m10 = -sin( angle );
        m11 = cos( angle );

        return *this;
    }

	matrix3 operator-() const
	{
		matrix3 M;
		M.m00 = -m00;
		M.m01 = -m01;
		M.m02 = -m02;
		M.m10 = -m10;
		M.m11 = -m11;
		M.m12 = -m12;
		M.m20 = -m20;
		M.m21 = -m21;
		M.m22 = -m22;
		return M;
	}

    float m00;
    float m01;
    float m02;
    float m10;
    float m11;
    float m12;
    float m20;
    float m21;
    float m22;
};

matrix3 operator*( matrix3 const& a, matrix3 const& b )
{
    matrix3 r;
    r.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20;
    r.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21;
    r.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22;
    r.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20;
    r.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21;
    r.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22;
    r.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20;
    r.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21;
    r.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22;
    return r;
}

matrix3 operator+( const matrix3& A, const matrix3& B ) {
	matrix3 R;
    R.m00 = A.m00 + B.m00;
    R.m01 = A.m01 + B.m01;
    R.m02 = A.m02 + B.m02;
    R.m10 = A.m10 + B.m10;
    R.m11 = A.m11 + B.m11;
    R.m12 = A.m12 + B.m12;
    R.m20 = A.m20 + B.m20;
    R.m21 = A.m21 + B.m21;
    R.m22 = A.m22 + B.m22;
	return R;
}

float lerp( float a, float b, float l )
{
    return a + ( b - a ) * l;
}

float2 lerp( float2 const& a, float2 const& b, float l )
{
    float2 result;
    result.x = lerp(a.x, b.x, l);
    result.y = lerp(a.y, b.y, l);
    return result;
}

float to_rads(float deg)
{
	return deg * 2.0f * 3.14159265f / 360.f;
}

float lerp_angle( float start_angle, float end_angle, float blend, int spin )
{
    if( spin < 0 )
    {
        if( end_angle > start_angle )
        {
            end_angle -= 360.f;
        }
    }
    else
    {
        if( end_angle < start_angle )
        {
            end_angle += 360.0f;
        }
    }

    float result = lerp( start_angle, end_angle, blend );
    result = to_rads(result);
	return result;
}

struct xml_writer
{
	std::vector<char const*> _tags;
	FILE* _file;
	bool _is_tag_open;

	xml_writer& begin_doc( char const* path )
	{
		_file = fopen( path, "w" );
		_tags.clear();
		_is_tag_open = false;

		return *this;
	}

	void end_doc()
	{
        fflush( _file );
		fclose( _file );		
	}

	xml_writer& push( char const* tag_name )
	{
		if( _is_tag_open )
		{
			fprintf( _file, ">\n" );
		}

		_tags.push_back( tag_name );
		
		print_indent();		

		fprintf( _file, "<%s", tag_name );			

		_is_tag_open = true;

		return *this;
	}

	xml_writer& attribute( char const* name, char const* value )
	{
		fprintf( _file, " %s=\"%s\"", name, value );	
		return *this;
	}

    xml_writer& attribute( char const* name, int value )
    {
        fprintf( _file, " %s=\"%i\"", name, value );
        return *this;
    }
    xml_writer& attribute( char const* name, float value )
    {
        fprintf( _file, " %s=\"%f\"", name, value );
        return *this;
    }


	xml_writer& pop( bool empty = false)
	{
		if( empty )
		{
			fprintf( _file, "/>\n" );
		}
		else
		{
			print_indent();
			fprintf( _file, "</%s>\n", _tags[_tags.size() - 1] );
		}

		_tags.pop_back();

		_is_tag_open = false;
		
		return *this;
	}

	void print_indent()
	{
		char tabs[256];
		for( int i = 0; i < (int)_tags.size() - 1; ++i )
		{
			tabs[i] = ' ';
		}
		tabs[max( (int)_tags.size() - 1, 0 )] = 0;
		fprintf( _file, tabs );
	}
};


/*
time_t get_last_modified( char const* path )
{
    struct stat info;
    int result = stat( path, &info );
    if( result == 0 )
    {
        return info.st_mtime;
    }
    else
    {
        return 0;
    }
    
}

bool is_more_recent( char const* path_a, char const* path_b )
{
    time_t a_time = get_last_modified( path_a );
    time_t b_time = get_last_modified( path_b );
    return a_time < b_time;
}

bool exists( char const* path )
{
	struct _stat info;
	int result = _stat( path, &info );
	return result == 0;
}

char const* get_file_name( char const* path )
{
	char const* name = strrchr( path, '/' );
	if( !name )
	{
		return path;		
	}
	return name;
}


void get_output_file_path( char const* input_file_path, char* output_file_path )
{
    strcpy( output_file_path, input_file_path );
    strcpy( strrchr( output_file_path, '.' ), ".zip" );
}
*/

char const* skip_slash( char const* name )
{
    if( name[0] == '/' )
    {
        ++name;
    }
    return name;
}

void import_frame(
    IN  s_file&   file,
    OUT int&      num,
    OUT int&      duration,
    OUT char*     image_path,
    OUT int2&     dimensions,
    OUT float2&   pivot,
    OUT int&      id
    )
{
    num = file.id;
    duration = 1;    
    strcpy(image_path, file.name.c_str());
    dimensions.x = file.width;
    dimensions.y = file.height;
    pivot.x = file.pivot_x;
    pivot.y = file.pivot_y;
    id = file.id;
}

void import_symbol(
    IN  s_folder&  folder,
    IN  int        symbol_index,
    OUT char*      symbol_name, 
    OUT int&       symbol_frame_count, 
    OUT int*       frame_nums, 
    OUT int*       frame_durations, 
    OUT char**     frame_image_paths, 
    OUT int2*      frame_dimensions,
    OUT float2*    frame_pivots,
    OUT symbol_id* ids
    )
{
    if(folder.name.size() > 0)
    {
        strcpy(symbol_name, skip_slash(folder.name.c_str()));
    }
    else
    {
        sprintf(symbol_name, "symbol%i", folder.id);
    }
    
    
    symbol_frame_count = folder.files.size();

    int frame_index = 0;
    for(s_file_map::iterator file_iter = folder.files.begin(); file_iter != folder.files.end(); ++file_iter)
    {
        symbol_id& id = ids[frame_index];
        id.folder = folder.id;

        s_file& file = *file_iter->second;
        import_frame(
            IN  file,
            OUT frame_nums[frame_index],
            OUT frame_durations[frame_index],
            OUT frame_image_paths[frame_index], 
            OUT frame_dimensions[frame_index], 
            OUT frame_pivots[frame_index],
            OUT id.file
            );
        ++frame_index;
    }
}

void import_build(
    IN  s_data&     scml,
    OUT char*       build_name,
    OUT char**      symbol_names, 
    OUT int*        symbol_frame_counts, 
    OUT int*        symbol_frame_indices, 
    OUT int*        frame_nums, 
    OUT int*        frame_durations, 
    OUT char**      frame_image_paths, 
    OUT int2*       frame_dimensions, 
    OUT float2*     frame_pivots,        
    OUT symbol_id*  ids  
    )
{
    strcpy(build_name, Path(scml.name).basename().c_str());

    int symbol_index = 0;
    int symbol_frame_index = 0;

    for(s_folder_map::iterator iter = scml.folders.begin(); iter != scml.folders.end(); ++iter)
    {
        s_folder& folder = *iter->second;        
		symbol_frame_indices[symbol_index] = symbol_frame_index;
        import_symbol(
            IN folder,
            IN symbol_index,
            OUT symbol_names[symbol_index], 
            OUT symbol_frame_counts[symbol_index], 
            OUT &frame_nums[symbol_frame_index],
            OUT &frame_durations[symbol_frame_index],
            OUT &frame_image_paths[symbol_frame_index], 
            OUT &frame_dimensions[symbol_frame_index],
            OUT &frame_pivots[symbol_frame_index], 
            OUT &ids[symbol_frame_index]
            );
        symbol_frame_index += symbol_frame_counts[symbol_index];
        symbol_index++;
    }
}

void convert_image_path_to_name(
    IN char const*    path,
    OUT char*   name
    )
{
    char const* name_with_ext = strrchr( path, '/' );
    if( !name_with_ext )
    {
        name_with_ext = path;       
    }
    else
    {
        ++name_with_ext;
    }

    int length = strlen(name_with_ext) + 1;
    char const* extension = strrchr( name_with_ext, '.' );
    if(0 != extension)
    {
        length = length - strlen(extension) - 1;
    }
    
    memcpy( name, name_with_ext, length );
    name[length] = 0;
}

void create_trans_rot_scale_pivot_matrix(
	IN  float2 const&	pos,
	IN  float			angle,
	IN  float2 const&	scale,
	IN  float2 const&	pivot,
	OUT matrix3& m
	)
{
#ifdef NEWPIVOTS
	/*
	 * Now the pivots are being properly placed as xy coordinates of the build symbol, within build.xml.
	 */
	(void)pivot;
#else
    matrix3 pivot_trans;
    pivot_trans.set_translation(pivot.x, pivot.y);

#endif

    matrix3 scale_mat;
    scale_mat.set_scale(scale.x, scale.y);

    matrix3 trans;
    trans.set_translation(pos.x, pos.y);

    matrix3 rot;
    rot.set_rotation(angle);

#ifdef NEWPIVOTS
	m = trans * rot * scale_mat; 
#else
	m = trans* rot * scale_mat * pivot_trans;
#endif
}


void convert_timeline_frame_to_element_frame(
    IN  int2 const&		image_dimensions,
    IN  float2 const&	pivot,   
    IN  float2 const&	scale,
    IN  float2 const&	pos,
    IN  float			angle,
    OUT matrix3&		m
    )
{
    float scaled_pivot_x = pivot.x * (float)image_dimensions.x;
    float scaled_pivot_y = pivot.y * (float)image_dimensions.y;

   create_trans_rot_scale_pivot_matrix(
	   IN float2(pos.x, -pos.y),
	   IN angle,
	   IN scale,
	   IN float2(image_dimensions.x / 2.f - scaled_pivot_x, -(image_dimensions.y / 2.f - scaled_pivot_y)),
	   OUT m
	   );
}

void convert_timeline_to_frames(
    IN  int				anim_length,
	IN  bool			looping,
    IN  int				frame_num, 
    IN  int				key_count,
    IN  int const*		times,
    IN  float2 const*	positions,
	IN  int2 const*		dimensions,
	IN  float2 const*	pivots,
	IN  float2 const*	scales,
	IN  float const*	angles,
	IN  int const*		spins,
	IN  float const*	alphas,
    OUT float2&			frame_position,
	OUT int2&			frame_dimension,
	OUT float2&			frame_pivot,
	OUT float2&			frame_scale,
	OUT float&			frame_angle,
	OUT float&			frame_alpha
    )
{
    int start_key = 0;
    int end_key = 0;
    for(int i = 0; i < key_count; ++i)
    {
        if(times[i] == frame_num)
        {
            start_key = i;
            end_key = i;
            break;
        }
        else if(times[i] < frame_num)
        {
            start_key = i;
            end_key = i;
        }
        else
        {
            end_key = i;
            break;
        }
    }

    int start_time = times[start_key];
    int end_time = times[end_key];

    if(start_key == key_count - 1 && looping && times[start_key] != frame_num)
    {
        end_key = 0;
        end_time = anim_length;
    }

    float blend = 0.f;
    if( end_time != start_time )
    {
        blend = ( float )( frame_num - start_time ) / ( float )( end_time - start_time );
    }

	frame_position = lerp(positions[start_key], positions[end_key], blend);
	frame_dimension = dimensions[start_key];
	frame_pivot = lerp(pivots[start_key], pivots[end_key], blend);
	frame_scale = lerp(scales[start_key], scales[end_key], blend);
	frame_angle = lerp_angle(angles[start_key], angles[end_key], blend, spins[end_key]);
	frame_alpha = lerp(alphas[start_key], alphas[end_key], blend);
}


void convert_anim_timelines_to_frames(
    IN  int     length,
	IN  bool	looping,
    IN  int     timeline_count,
    IN  int*    timeline_key_start_indices,
    IN  int*    timeline_key_counts,
    IN  int*    timeline_key_times,
    IN  float2* timeline_key_positions,
	IN  int2*   timeline_key_dimensions,
	IN  float2* timeline_key_pivots,
	IN  float2* timeline_key_scales,
	IN  float*  timeline_key_angles,
	IN  int*	timeline_key_z_indices,
	IN  int*	timeline_key_spins,
	IN  int*	timeline_key_symbol_frame_nums,
	IN  char**	timeline_key_names,
	IN  int*	timeline_key_parent_ids,
	IN	float*	timeline_key_alphas,
	OUT int&	anim_frame_count,
	OUT int*	frame_element_count,
	OUT int*    frame_element_start_indices,
	OUT int*	frame_indices,
	OUT float2* frame_positions,
	OUT int2*	frame_dimensions,
	OUT int*    timeline_frame_symbol_frame_nums,
	OUT char**  timeline_frame_names,
	OUT char**  timeline_frame_layer_names,
    OUT float2* timeline_frame_positions,
	OUT int2*	timeline_frame_dimensions,
	OUT float2* timeline_frame_pivots,
	OUT float2*	timeline_frame_scales,
	OUT float*	timeline_frame_angles,
	OUT int*	timeline_frame_z_indices,
	OUT int*	timeline_frame_parent_ids,
	OUT float*	timeline_frame_alphas,
	OUT int&	element_index
    )
{
    int frame_index = 0;
    for(int frame_num = 0; frame_num < length; frame_num+=1000/FRAME_RATE)
    {
		frame_element_start_indices[frame_index] = element_index;
        for(int i = 0; i < timeline_count; ++i)
        {
            int key_start_index = timeline_key_start_indices[i];

            convert_timeline_to_frames(
				length,
				looping,
                frame_num, 
                timeline_key_counts[i],
                &timeline_key_times[key_start_index],
                &timeline_key_positions[key_start_index],
				&timeline_key_dimensions[key_start_index],
				&timeline_key_pivots[key_start_index],
				&timeline_key_scales[key_start_index],
				&timeline_key_angles[key_start_index],
				&timeline_key_spins[key_start_index],
				&timeline_key_alphas[key_start_index],
                timeline_frame_positions[element_index],
				timeline_frame_dimensions[element_index],
				timeline_frame_pivots[element_index],
				timeline_frame_scales[element_index],
				timeline_frame_angles[element_index],
				timeline_frame_alphas[element_index]
				);
			char timeline_layer_name[1024];
			sprintf(timeline_layer_name, "timeline_%i", i);
			strcpy(timeline_frame_names[element_index], timeline_key_names[key_start_index]);
			strcpy(timeline_frame_layer_names[element_index], timeline_layer_name);
			timeline_frame_z_indices[element_index] = timeline_key_z_indices[key_start_index];
			timeline_frame_symbol_frame_nums[element_index] = timeline_key_symbol_frame_nums[key_start_index];
			timeline_frame_parent_ids[element_index] = timeline_key_parent_ids[key_start_index];

			frame_dimensions[frame_index] = timeline_key_dimensions[key_start_index];
			++element_index;
        }
		frame_positions[frame_index].x = 0;
		frame_positions[frame_index].y = 0;		
		frame_element_count[frame_index] = timeline_count;	
		frame_indices[frame_index] = frame_index;        
		++frame_index;
    }
	anim_frame_count = frame_index;
}

void export_symbol_frame(
	OUT symbol_frame_metadata_t& frame_metadata,
    IN xml_writer&  writer,
    IN int				num,
    IN int				duration,
    IN char const*		image,
    IN int2 const&		dimensions,
    IN float2 const&	position
    )
{
	frame_metadata.x = position.x;
	frame_metadata.y = position.y;
	frame_metadata.w = dimensions.x;
	frame_metadata.h = dimensions.y;

    writer.push("Frame")
        .attribute( "framenum", num )
        .attribute( "duration", duration )
        .attribute( "image", image )
        .attribute( "w", dimensions.x )
        .attribute( "h", dimensions.y )
        .attribute( "x", position.x )
        .attribute( "y", position.y )
    .pop( true );    
}

void export_symbol(
	OUT symbol_metadata_t&	symbol_metadata,
    IN xml_writer&			writer, 
    IN char const*			name,
    IN int					frame_count, 
    IN int const*			frame_nums, 
    IN int const*			frame_durations,
    IN char const* const*	frame_images,
    IN int2 const*			frame_dimensions,
    IN float2 const*		frame_positions
    )
{
    writer.push("Symbol")
        .attribute("name", skip_slash(name));

		symbol_metadata.resize(frame_count);

        for(int i = 0; i < frame_count; ++i)
        {
			symbol_frame_metadata_t& frame_metadata = symbol_metadata[i];

            export_symbol_frame(
				OUT frame_metadata,
                IN writer,
                IN frame_nums[i],
                IN frame_durations[i],
                IN frame_images[i],
                IN frame_dimensions[i],
                IN frame_positions[i]
                );
        }
    writer.pop();        
}

void export_build(
	OUT build_metadata_t&	build_metadata,
    IN char const*			build_xml_path,
    IN char const*			build_name,
    IN int					symbol_count, 
    IN char const* const*	symbol_names, 
    IN int const*			symbol_frame_counts, 
    IN int const*			symbol_frame_indices, 
    IN int const*			frame_nums, 
    IN int const*			frame_durations, 
    IN char const * const*	frame_images, 
    IN int2 const*			frame_dimensions, 
    IN float2 const*		frame_positions
    )
{
    xml_writer writer;

    writer.begin_doc(build_xml_path);
    writer.push("Build")
        .attribute("name", build_name);

        for(int i = 0; i < symbol_count; ++i)
        {                
            int frame_index = symbol_frame_indices[i];

			symbol_metadata_t& symbol_metadata = build_metadata[symbol_names[i]];

            export_symbol(
				OUT symbol_metadata,
                IN writer, 
                IN symbol_names[i], 
                IN symbol_frame_counts[i], 
                IN &frame_nums[frame_index], 
                IN &frame_durations[frame_index], 
                IN &frame_images[frame_index],
                IN &frame_dimensions[frame_index],
                IN &frame_positions[frame_index]
                );
        }

    writer.pop();
    writer.end_doc();            
}

void get_build_counts(
    IN  s_data& scml,
    OUT int&    symbol_count,
    OUT int&    symbol_frame_count
    )
{
    for(s_folder_map::iterator iter = scml.folders.begin(); iter != scml.folders.end(); ++iter)
    {
        ++symbol_count;
        s_folder& folder = *iter->second;
        for(s_file_map::iterator file_iter = folder.files.begin(); file_iter != folder.files.end(); ++file_iter)
        {
            ++symbol_frame_count;
        }
    }
}

void export_element(
    IN xml_writer&  writer,
    IN const char*        name,
    IN const char*        layer_name, 
    IN int          frame,
    IN int          z_index,
    IN const matrix3&     m
    )
{
    writer.push( "element" )
        .attribute( "name", name )
        .attribute( "layername", layer_name )
        .attribute( "frame", frame )
        .attribute( "z_index",  z_index )
        .attribute( "m_a", m.m00 )
        .attribute( "m_b", m.m10 )
        .attribute( "m_c", m.m01 )
        .attribute( "m_d", m.m11 )
        .attribute( "m_tx", m.m02 )
        .attribute( "m_ty", m.m12 );
    writer.pop( true );
}

// Per element.
static bool extend_bounding_box(
	IN const symbol_metadata_t& symbol_metadata,
	IN const char * symbol_name,
	IN bool first,
	OUT rectangle& rect,
	IN int frame,
	IN const matrix3& m
	)
{
	if(symbol_metadata.size() <= frame) {
		log_and_fprint(stderr, "WARNING: frame %0d of animation symbol %s is being used by the animation but not defined by the build.\n");
		return false;
	}

	const symbol_frame_metadata_t& frame_metadata = symbol_metadata[frame];

	float x1, x2, y1, y2;

	x1 = m.m00*frame_metadata.x + m.m01*frame_metadata.y + m.m02;
	x2 = x1 + m.m00*frame_metadata.w + m.m01*frame_metadata.h;

	y1 = m.m10*frame_metadata.x + m.m11*frame_metadata.y + m.m12;
	y2 = x1 + m.m10*frame_metadata.w + m.m11*frame_metadata.h;

	if(x2 < x1) {
		std::swap(x1, x2);
	}
	if(y2 < y1) {
		std::swap(y1, y2);
	}

	if(first || x1 < rect.x1) {
		rect.x1 = x1;
	}
	if(first || x2 > rect.x2) {
		rect.x2 = x2;
	}
	if(first || y1 < rect.y1) {
		rect.y1 = y1;
	}
	if(first || y2 > rect.y2) {
		rect.y2 = y2;
	}

	return true;
}

void export_animation_frame(
	IN const build_metadata_t& build_metadata,
    IN xml_writer&  writer,
    IN int          index,
    IN int2&        dimensions,
    IN float2&      position,
    IN int          element_count,
    IN char**       element_names,
    IN char**       element_layer_names,
    IN int*         element_frames,
    IN int*         element_z_indices,
	IN float*		element_alphas,
    IN matrix3*     element_matrices
    )
{
	rectangle bounding_rectangle;
	bool uninitialized_rect = true;

	for(int i = 0; i < element_count; ++i)
	{
		if(element_alphas[i] >= 0.5)
		{
			build_metadata_t::const_iterator symbol_searcher = build_metadata.find(element_names[i]);
			if(symbol_searcher != build_metadata.end()) {
				if(extend_bounding_box(
					IN symbol_searcher->second,
					IN element_names[i],
					IN uninitialized_rect,
					IN bounding_rectangle,
					IN element_frames[i],
					IN element_matrices[i]
					))
				{
					uninitialized_rect = false;
				}
			}
		}
	}

	bounding_box(bounding_rectangle).split( position, dimensions );

    writer.push( "frame" )
        .attribute( "idx", index * 1000 / FRAME_RATE )
        .attribute( "w", dimensions.x )
        .attribute( "h", dimensions.y )
        .attribute( "x", position.x )
        .attribute( "y", position.y );

        for(int i = 0; i < element_count; ++i)
        {
			if(element_alphas[i] >= 0.5)
			{
				export_element(
					IN writer,
					IN element_names[i],
					IN element_layer_names[i],
					IN element_frames[i],
					IN element_z_indices[i],
					IN element_matrices[i]
					);
			}
         }

    writer.pop();
}

void export_animation(
	IN const build_metadata_t& build_metadata,
    IN xml_writer&  writer,
    IN char*        name,
    IN char*        root,
    IN int          num_frames,
    IN int          framerate,
    IN int*         frame_indices,
    IN int2*        frame_dimensions,
    IN float2*      frame_positions,
    IN int*         frame_element_counts,
    IN int*         frame_element_start_indices,
    IN char**       element_names,
    IN char**       element_layer_names,
    IN int*         element_frames,
    IN int*         element_z_indices,
	IN float*		element_alphas,
    IN matrix3*     element_matrices
    )
{
    writer.push( "anim" )
        .attribute( "name", name )
        .attribute( "root", root )
        .attribute( "numframes", num_frames * 1000 / FRAME_RATE )
        .attribute( "framerate", framerate );

    for(int i = 0; i < num_frames; ++i)
    {
        int element_start_index = frame_element_start_indices[i];
        export_animation_frame(
			IN build_metadata,
            IN writer,
            IN frame_indices[i],
            IN frame_dimensions[i],
            IN frame_positions[i],
            IN frame_element_counts[i],
            IN &element_names[element_start_index],
            IN &element_layer_names[element_start_index],
            IN &element_frames[element_start_index],
            IN &element_z_indices[element_start_index],
			IN &element_alphas[element_start_index],
            IN &element_matrices[element_start_index]
            );
    }

    writer.pop();
}

bool is_bone_timeline(
	IN s_timeline& timeline
	)
{
	return strcmp(timeline.object_type.c_str(), "bone") == 0;
}

void import_animation(
    IN s_animation& anim,
    OUT char*       name,
    OUT int&        num_frames,
    OUT int&        framerate,
	OUT bool&		looping
    )
{
    strcpy(name, anim.name.c_str());
    num_frames = anim.length;
    framerate = FRAME_RATE;
	looping = strcmp( anim.looping.c_str(), "true" ) == 0;
}

void import_mainline_key(
    s_object_ref&   object,
    OUT float2&     position,
    OUT float2&     pivot,
    OUT float&      angle,
    OUT float2&     scale,
    OUT int&        z_index,
    OUT int&        spin,
    OUT int&        id,
    OUT int&        timeline_id,
	OUT int&		parent_id
    )
{
    position.x = object.abs_x;
    position.y = object.abs_y;
    scale.x = object.abs_scale_x;
    scale.y = object.abs_scale_y;
    angle = object.abs_angle;
    z_index = object.z_index;
    spin = 0;

    if(object.found_pivot_x)
    {
        pivot.x = object.abs_pivot_x;
    }
    if(object.found_pivot_y)
    {
        pivot.y = object.abs_pivot_y;
    }

    id = object.key;
    timeline_id = object.timeline;
	parent_id = object.parent;
}

bool is_valid_animation(s_animation& anim)
{
    return anim.length > 1000 / FRAME_RATE && anim.timelines.size() > 0;
}

bool is_valid_timeline(
	IN s_timeline& timeline
	)
{
	return strcmp(timeline.object_type.c_str(), "bone") != 0;
}

void import_mainline(
    IN  s_data& scml,
    OUT int*    mainline_key_start_indices,
    OUT int*    mainline_key_counts,
    OUT float2* mainline_key_positions,
    OUT float2* mainline_key_pivots,
    OUT float*  mainline_key_angles,
    OUT float2* mainline_key_scales,
    OUT int*    mainline_key_z_indices,
    OUT int*    mainline_key_spins,
    OUT int*    mainline_key_ids,
    OUT int*    mainline_key_timeline_ids,   
	OUT int*    parent_ids
    )
{
    int anim_index = 0;
    int mainline_key_index = 0;
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        { 
            s_animation& anim = *anim_iter->second;
            if(is_valid_animation(anim))
            {
                s_mainline_key_map& keys = anim.mainline.keys;                
                mainline_key_start_indices[anim_index] = mainline_key_index;

                for(s_mainline_key_map::iterator iter = keys.begin(); iter != keys.end(); ++iter)
                {
                    s_mainline_key& key = *iter->second;
                    for(s_object_container_map::iterator object_iter = key.objects.begin(); object_iter != key.objects.end(); ++object_iter )
                    {
                        s_object_ref& object = *object_iter->second.object_ref;
                        import_mainline_key(
                            IN  object,
                            OUT mainline_key_positions[mainline_key_index],
                            OUT mainline_key_pivots[mainline_key_index],
                            OUT mainline_key_angles[mainline_key_index],
                            OUT mainline_key_scales[mainline_key_index],
                            OUT mainline_key_z_indices[mainline_key_index],
                            OUT mainline_key_spins[mainline_key_index],
                            OUT mainline_key_ids[mainline_key_index],
                            OUT mainline_key_timeline_ids[mainline_key_index],
							OUT parent_ids[mainline_key_index]
                            );						
						
                        mainline_key_index++;
                    }
                }
                mainline_key_counts[anim_index] = mainline_key_index - mainline_key_start_indices[anim_index];

                ++anim_index;
            }
        }
    }
}

void find_mainline_key_index(
    IN  int& base_index,
    IN  int& timeline_id,
    IN  int& key_id,
    IN  int& key_count,
    IN  int* key_ids,
    IN  int* timeline_ids,
    OUT int& key_index
    )
{
    key_index = base_index;
    for(int i = 0; i < key_count; ++i)
    {
        if(timeline_id == timeline_ids[i] && key_id == key_ids[i])
        {
            key_index = base_index + i;
            break;
        }
    }
}

void find_symbol_index(
    IN symbol_id&   id,
    IN int          count,
    IN symbol_id*   ids,
    OUT int&        index
    )
{
    index = 0;
    for(int i = 0; i < count; ++i)
    {
        if(id.folder == ids[i].folder && id.file == ids[i].file)
        {
            index = i;
            break;
        }
    }
}

void import_timeline_maps(
    IN  s_data&     scml,
    IN  int*        mainline_key_start_indices,
    IN  int*        mainline_key_counts,
    IN  int*        mainline_key_ids,
    IN  int*        mainline_key_timeline_ids,
    IN  int         symbol_frame_count,
    IN  symbol_id*  symbol_ids,
    OUT int*        timeline_mainline_first_key_map,
    OUT int*        timeline_mainline_key_map,
    OUT int*        timeline_symbol_map
    )
{
    int timeline_key_index = 0;
    int anim_index = 0;
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        { 
            s_animation& anim = *anim_iter->second;
#ifdef ANIMDEBUG
			cout << "Animation: " << anim.name << endl;
#endif
            if(is_valid_animation(anim))
            {                
                for(s_timeline_map::iterator timeline_iter = anim.timelines.begin(); timeline_iter != anim.timelines.end(); ++timeline_iter)
                { 
#ifdef ANIMDEBUG
					cout << "\tTimeline: " << timeline_iter->first << endl;
#endif
					s_timeline& timeline = *timeline_iter->second;
					if(is_valid_timeline(timeline))
					{
						int first_mainline_key_index = 0;
						int mainline_start_index = mainline_key_start_indices[anim_index];
						find_mainline_key_index(
							IN  mainline_start_index,
							IN  timeline.id,
							IN  timeline.keys.begin()->second->id,                            
							IN  mainline_key_counts[anim_index],
							IN  &mainline_key_ids[mainline_start_index],
							IN  &mainline_key_timeline_ids[mainline_start_index],
							OUT first_mainline_key_index
							); 

						int first_key_id = timeline.keys.begin()->second->id;
						for(s_timeline_key_map::iterator key_iter = timeline.keys.begin(); key_iter != timeline.keys.end(); ++key_iter)
						{
#ifdef ANIMDEBUG
							cout << "\t\tKey: " << key_iter->first << endl;
#endif
							s_timeline_key& key = *key_iter->second;
							s_timeline_object& object = key.object;
							int mainline_start_index = mainline_key_start_indices[anim_index];

							find_mainline_key_index(
								IN  mainline_start_index,
								IN  timeline.id,
								IN  key.id,                            
								IN  mainline_key_counts[anim_index],
								IN  &mainline_key_ids[mainline_start_index],
								IN  &mainline_key_timeline_ids[mainline_start_index],
								OUT timeline_mainline_key_map[timeline_key_index]
								);

							timeline_mainline_first_key_map[timeline_key_index] = first_mainline_key_index;

							symbol_id sym_id;
							sym_id.folder = object.folder;
							sym_id.file = object.file;
							find_symbol_index(
								sym_id,
								symbol_frame_count,
								symbol_ids,
								timeline_symbol_map[timeline_key_index]
								);

#ifdef ANIMDEBUG
							const size_t sym_index = timeline_symbol_map[timeline_key_index];
							cout << "Got symbol index " << timeline_symbol_map[timeline_key_index] << " (folder: " << symbol_ids[sym_index].folder << ", file: " << symbol_ids[sym_index].file << ")" << endl;
#endif

							++timeline_key_index;
						}
					}
                }
                ++anim_index;
            }
        }
    }    
}

void import_timeline_key(
    IN s_timeline_key&  key,
    OUT float2&         position,
    OUT float2&         pivot,
    OUT float&          angle,
    OUT float2&         scale,
    OUT int&            spin,
	OUT int&            time,
	OUT int&			symbol_frame_num,
	OUT float&			alpha
    )
{
    s_timeline_object& object = key.object;

    spin = key.spin;
    angle = object.angle;
	alpha = object.a;
    scale.x = object.scale_x;
    scale.y = object.scale_y;
    
	time = key.time;
	// LOOK AT ME
	symbol_frame_num = object.file;

    position.x = object.x;
    position.y = object.y;

    if(object.found_pivot_x)
    {
        pivot.x = object.pivot_x;
    }
    if(object.found_pivot_y)
    {
        pivot.y = object.pivot_y;
    }
 }

void import_timelines(
    IN  s_data& scml,
	OUT int*	anim_timeline_counts,
	OUT int*	anim_timeline_start_indices,
	OUT int*	anim_timeline_key_start_indices,
    OUT int*	anim_timeline_key_counts,
    OUT float2* timeline_key_positions,
    OUT float2* timeline_key_pivots,
    OUT float*  timeline_key_angles,
    OUT float2* timeline_key_scales,
    OUT int*    timeline_key_spins,
	OUT int*	timeline_key_times,
	OUT int*	timeline_key_symbol_frame_nums,
	OUT char**  timeline_key_names,
	OUT float*	timeline_key_alphas
    )
{
    int anim_index = 0;
    int timeline_key_index = 0;
	int timeline_index = 0;
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        { 
            s_animation& anim = *anim_iter->second;
            if(is_valid_animation(anim))
            {
				anim_timeline_start_indices[anim_index] = timeline_index;
				anim_timeline_counts[anim_index] = 0;
                for(s_timeline_map::iterator timeline_iter = anim.timelines.begin(); timeline_iter != anim.timelines.end(); ++timeline_iter)
                {					
                    s_timeline& timeline = *timeline_iter->second;
					if(is_valid_timeline(timeline))
					{
						anim_timeline_key_start_indices[timeline_index] = timeline_key_index;
						anim_timeline_key_counts[timeline_index] = timeline.keys.size();					
						for(s_timeline_key_map::iterator key_iter = timeline.keys.begin(); key_iter != timeline.keys.end(); ++key_iter)
						{
							s_timeline_key& key = *key_iter->second;                        
						
							import_timeline_key(
								IN  key,
								OUT timeline_key_positions[timeline_key_index],
								OUT timeline_key_pivots[timeline_key_index],
								OUT timeline_key_angles[timeline_key_index],
								OUT timeline_key_scales[timeline_key_index],
								OUT timeline_key_spins[timeline_key_index],
								OUT timeline_key_times[timeline_key_index],
								OUT timeline_key_symbol_frame_nums[timeline_key_index],
								OUT timeline_key_alphas[timeline_key_index]
								);
							
                            s_folder& folder = *scml.folders.find(key.object.folder)->second;
                            if(folder.name.size() > 0)
                            {
                                strcpy(timeline_key_names[timeline_key_index], skip_slash(folder.name.c_str()));
                            }
                            else
                            {
                                sprintf(timeline_key_names[timeline_key_index], "symbol%i", folder.id);
                            }

							++timeline_key_index;
						}
						++timeline_index;
						++anim_timeline_counts[anim_index];
					}
                }
                ++anim_index;
            }
        }    
    }
}

struct bone
{
	int		id;
	float	x;
	float	y;
	float	scale_x;
	float	scale_y;
	float	angle;
	int		time;
	int		spin;
	int		parent_id;
	bool	is_flattened;
	matrix3 flattened;
};


void flatten(
	IN  bone& b, 
	IN  int bone_count,
	OUT bone* bones
	)
{
	if(b.is_flattened || b.parent_id == -1)
	{
		return;
	}

	for(int parent_bone_index = 0; parent_bone_index < bone_count; ++parent_bone_index)
	{
		bone& parent = bones[parent_bone_index];
		if(parent.id == b.parent_id)
		{
			flatten(parent, bone_count, bones);
			b.scale_x*=parent.scale_x;
			b.scale_y*=parent.scale_y;

			float sx = b.x * parent.scale_x;
			float sy = b.y * parent.scale_y;
			float s = sin(parent.angle);
			float c = cos(parent.angle);
			b.x= parent.x + sx*c - sy*s;
			b.y= parent.y + sx*s + sy*c;
			b.angle+=parent.angle;
			break;		
		}
	}
	b.is_flattened = true;
}

void import_bones(
    IN  s_data& scml,
	OUT int*	anim_bone_counts,
	OUT int*	anim_bone_start_indices,
	OUT int*	anim_bone_key_start_indices,
    OUT int*	anim_bone_key_counts,
	OUT bone*	bones
	)
{
    int anim_index = 0;
    int timeline_key_index = 0;
	int timeline_index = 0;
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        { 
            s_animation& anim = *anim_iter->second;
            if(is_valid_animation(anim))
            {
				anim_bone_start_indices[anim_index] = timeline_index;
				anim_bone_counts[anim_index] = 0;
                for(s_timeline_map::iterator timeline_iter = anim.timelines.begin(); timeline_iter != anim.timelines.end(); ++timeline_iter)
                {					
                    s_timeline& timeline = *timeline_iter->second;
					if(is_bone_timeline(timeline))
					{
						anim_bone_key_start_indices[timeline_index] = timeline_key_index;
						anim_bone_key_counts[timeline_index] = timeline.keys.size();					
						for(s_timeline_key_map::iterator key_iter = timeline.keys.begin(); key_iter != timeline.keys.end(); ++key_iter)
						{
							s_timeline_key& key = *key_iter->second;                        
						
							bone& b = bones[timeline_key_index];
							b.x = key.bone.x;
							b.y = key.bone.y;
							b.scale_x = key.bone.scale_x;
							b.scale_y = key.bone.scale_y;
							b.angle = key.bone.angle;
							b.spin = key.spin;
							b.time = key.time;

							b.id = -1;		
							b.parent_id = -1;

							bool found_bone = false;
							for(s_bone_container_map::iterator mainline_bone_iter = anim.mainline.keys.begin()->second->bones.begin(); mainline_bone_iter != anim.mainline.keys.begin()->second->bones.end(); ++mainline_bone_iter)
							{
								if(mainline_bone_iter->second.bone_ref->timeline == timeline.id)
								{
									b.id = mainline_bone_iter->second.bone_ref->id;
									b.parent_id = mainline_bone_iter->second.bone_ref->parent;
									found_bone = true;
									break;
								}
							}
							if(!found_bone)
							{
								error("ERROR: Bone '%s' doesn't exist on first key frame.", timeline.name.c_str());
							}
							
							++timeline_key_index;
						}
						++timeline_index;
						++anim_bone_counts[anim_index];
					}
                }
                ++anim_index;
            }
        }    
    }
}

void import_animations(
    IN  s_data&     scml,
    OUT char**      names,
    OUT char**      roots,
    OUT int*        num_frames,
    OUT int*        framerates,
	OUT bool*       loopings
    )
{
    int anim_index = 0;
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        { 
            s_animation& anim = *anim_iter->second;
            if(is_valid_animation(anim))
            {
				strcpy(roots[anim_index], entity.name.c_str());
                import_animation(
                    anim,
                    names[anim_index],
                    num_frames[anim_index],
                    framerates[anim_index],
					loopings[anim_index]
                    );
                ++anim_index;
            }
        }
    }    
}

int get_anim_frame_count(int length)
{
	return 1 + (length - 1) / (1000/FRAME_RATE);
}

void get_anim_counts(
    IN  s_data& scml,
    OUT int&    anim_count,
    OUT int&    frame_count,
    OUT int&    element_count,
	OUT int&	timeline_count,
    OUT int&    mainline_key_count,
    OUT int&    timeline_key_count,
	OUT int&	bone_count,
	OUT int&	bone_key_count,
	OUT int&	bone_frame_count
    )
{
    for(s_entity_map::iterator iter = scml.entities.begin(); iter != scml.entities.end(); ++iter)
    {
        s_entity& entity = *iter->second;
        for(s_animation_map::iterator anim_iter = entity.animations.begin(); anim_iter != entity.animations.end(); ++anim_iter)
        {             
            s_animation& anim = *anim_iter->second;
            if(is_valid_animation(anim))
            {
                ++anim_count;
                int anim_frame_count = get_anim_frame_count(anim.length);
                frame_count += anim_frame_count;
                

                for(s_mainline_key_map::iterator iter = anim.mainline.keys.begin(); iter != anim.mainline.keys.end(); ++iter)
                {
                    s_mainline_key& key = *iter->second;
                    mainline_key_count +=  key.objects.size();
                }

				for(s_timeline_map::iterator iter = anim.timelines.begin(); iter != anim.timelines.end(); ++iter)
				{
					s_timeline& timeline = *iter->second;
					if(is_valid_timeline(timeline))
					{
						++timeline_count;
						timeline_key_count += timeline.keys.size();
						element_count += anim_frame_count;
					}
					else if(is_bone_timeline(timeline))
					{
						++bone_count;
						bone_key_count += timeline.keys.size();
						bone_frame_count += anim_frame_count;
					}
					
				}
            }            
        }
    }
}

void apply_mainline_keys(
	IN  int  timeline_key_count,
	IN  int* timeline_mainline_key_map,
	IN  int* mainline_key_z_indices,
	OUT int* timeline_key_z_indices
	)
{
    for(int i = 0; i < timeline_key_count; ++i)
    {
		timeline_key_z_indices[i] = mainline_key_z_indices[timeline_mainline_key_map[i]];
	}
}

void apply_mainline_keys(
    IN  int     timeline_key_count,
    IN  int*    timeline_mainline_key_map,
    IN  float2* mainline_key_positions,
    IN  float2* mainline_key_pivots,
    IN  float*  mainline_key_angles,
    IN  float2* mainline_key_scales,
    IN  int*    mainline_key_z_indices,
    IN  int*    mainline_key_spins,
    OUT float2* timeline_key_positions,
    OUT float2* timeline_key_pivots,
    OUT float*  timeline_key_angles,
    OUT float2* timeline_key_scales,
    OUT int*    timeline_key_z_indices,
    OUT int*    timeline_key_spins        
    )
{
    for(int i = 0; i < timeline_key_count; ++i)
    {
        timeline_key_positions[i] = mainline_key_positions[timeline_mainline_key_map[i]];
        timeline_key_pivots[i] = mainline_key_pivots[timeline_mainline_key_map[i]];
        timeline_key_angles[i] = mainline_key_angles[timeline_mainline_key_map[i]];
        timeline_key_scales[i] = mainline_key_scales[timeline_mainline_key_map[i]];
        timeline_key_z_indices[i] = mainline_key_z_indices[timeline_mainline_key_map[i]];
        timeline_key_spins[i] = mainline_key_spins[timeline_mainline_key_map[i]];
    }
}

void apply_mainline_parent_ids(
	IN int  timeline_key_count,
	IN int* timeline_mainline_key_map,
	IN int* mainline_parent_ids,
	IN int* timeline_key_parent_ids
	)
{
	for(int i = 0; i < timeline_key_count; ++i)
	{
		timeline_key_parent_ids[i] = mainline_parent_ids[timeline_mainline_key_map[i]];
	}
}

void apply_symbols(
    IN  float2*	symbol_frame_pivots,
	IN  int2*	symbol_frame_dimensions,
    IN  int*	timeline_symbol_map,
    IN  int		timeline_key_count,
    OUT float2* timeline_key_pivots,
	OUT int2*   timeline_key_dimensions
    )
{
    for(int i = 0; i < timeline_key_count; ++i)
    {
        timeline_key_pivots[i] = symbol_frame_pivots[timeline_symbol_map[i]];
		timeline_key_dimensions[i] = symbol_frame_dimensions[timeline_symbol_map[i]];
    }
}

char** allocate_strings(int string_count, int string_length)
{
	char** ptrs = new char*[string_count];
	char* string_buffer = new char[string_count * string_length];
	for(int i = 0; i < string_count; ++i)
	{
		ptrs[i] = string_buffer + i * string_length;
	}

	return ptrs;
}

void convert_z_index(
	IN int max_z,
	OUT int& z
	)
{
	z = max_z - z;
}

void convert_pivot(
    IN  int2&    image_dimensions,
    IN  float2&  input_pivot,
    OUT float2&  output_pivot
    )
{
    float scaled_pivot_x = input_pivot.x * (float)image_dimensions.x;
    float scaled_pivot_y = input_pivot.y * (float)image_dimensions.y;

    output_pivot.x = image_dimensions.x / 2.f - scaled_pivot_x;
    output_pivot.y = image_dimensions.y / 2.f - scaled_pivot_y;
    output_pivot.y = -output_pivot.y;
}

void convert_to_swappable_build(
    IN  int     timeline_key_count,
    IN  int*    timeline_symbol_map,
    IN  int*    timeline_key_times,
    IN  int2*   timeline_key_dimensions,
    IN  float2* timeline_key_pivots,
    OUT float2* symbol_frame_pivots
    )
{
    for(int i = 0; i < timeline_key_count; ++i)
    {
        if(timeline_key_times[i] == 0)
        {
            convert_pivot(
                IN  timeline_key_dimensions[i],
                IN  timeline_key_pivots[i],
                OUT symbol_frame_pivots[timeline_symbol_map[i]]
                );        
        }
    }
}

void build_scml(
    IN s_data& scml,
	OUT char**& image_paths,
	OUT int& image_path_count
    )
{
    // import build
    int symbol_count = 0;
    int symbol_frame_count = 0;

    get_build_counts(
        IN  scml,
        OUT symbol_count,
        OUT symbol_frame_count
        );

    char*       build_name = new char[MAX_NAME_LENGTH];
    char**      symbol_names = allocate_strings(symbol_count, MAX_NAME_LENGTH);
    int*        symbol_frame_counts = new int[symbol_count];
    int*        symbol_frame_indices = new int[symbol_count]; 
    int*        symbol_frame_nums = new int[symbol_frame_count];
    int*        symbol_frame_durations = new int[symbol_frame_count];
    char**      symbol_frame_image_paths = allocate_strings(symbol_frame_count, MAX_NAME_LENGTH);
    char**      symbol_frame_image_names = allocate_strings(symbol_frame_count, MAX_NAME_LENGTH);
    int2*       symbol_frame_dimensions = new int2[symbol_frame_count];
    float2*     symbol_frame_pivots = new float2[symbol_frame_count];
    symbol_id*  symbol_ids = new symbol_id[symbol_frame_count];

    import_build(
        IN  scml,
        OUT build_name,
        OUT symbol_names, 
        OUT symbol_frame_counts, 
        OUT symbol_frame_indices, 
        OUT symbol_frame_nums, 
        OUT symbol_frame_durations, 
        OUT symbol_frame_image_paths, 
        OUT symbol_frame_dimensions, 
        OUT symbol_frame_pivots, 
        OUT symbol_ids
        );

    // import animations
    int anim_count = 0;
    int frame_count = 0;
    int element_count = 0;
	int timeline_count = 0;
    int mainline_key_count = 0;
    int timeline_key_count = 0;
	int bone_count = 0;
	int bone_key_count = 0;
	int bone_frame_count = 0;

    get_anim_counts(
        IN  scml,
        OUT anim_count,
        OUT frame_count,
        OUT element_count,
		OUT timeline_count,
        OUT mainline_key_count,
        OUT timeline_key_count,
		OUT bone_count,
		OUT bone_key_count,
		OUT bone_frame_count
        );

    char** anim_names = allocate_strings(anim_count, MAX_NAME_LENGTH);
    char** anim_roots = allocate_strings(anim_count, MAX_NAME_LENGTH);
	int* anim_lengths = new int[anim_count];
    int* anim_framerates = new int[anim_count];
	bool* anim_loopings = new bool[anim_count];
    int* mainline_key_start_indices = new int[anim_count]; 
    int* mainline_key_counts = new int[anim_count];
	int* anim_bone_counts = new int[anim_count];
	int* anim_timeline_counts = new int[anim_count];
	int* anim_timeline_start_indices = new int[anim_count];
	int* anim_frame_start_indices = new int[anim_count];	
	int* anim_bone_start_indices = new int[anim_count];
	int* anim_frame_counts = new int[anim_count];
	int* anim_bone_frame_start_indices = new int[anim_count];
	int* anim_timeline_key_start_indices= new int[timeline_count];
	int* anim_timeline_key_counts = new int[timeline_count];

    int* frame_indices = new int[frame_count];
    int2* frame_dimensions = new int2[frame_count];
    float2* frame_positions = new float2[frame_count];
    int* frame_element_counts = new int[frame_count];
    int* frame_element_start_indices = new int[frame_count];
    matrix3* element_matrices = new matrix3[element_count];

    float2* mainline_key_positions = new float2[mainline_key_count];
    float2* mainline_key_pivots = new float2[mainline_key_count];
    float* mainline_key_angles = new float[mainline_key_count];
    float2* mainline_key_scales = new float2[mainline_key_count];
    int* mainline_key_z_indices = new int[mainline_key_count];
    int* mainline_key_spins = new int[mainline_key_count];
    int* mainline_ids = new int[mainline_key_count];
    int* mainline_key_ids = new int[mainline_key_count];
	int* mainline_key_timeline_ids = new int[mainline_key_count];
	int* mainline_parent_ids = new int[mainline_key_count];

	int* anim_bone_key_start_indices = new int[bone_count];	
	int* anim_bone_key_counts = new int[bone_count];
	bone* anim_bone_keys = new bone[bone_key_count];

    float2* timeline_key_positions = new float2[timeline_key_count];
    float2* timeline_key_pivots = new float2[timeline_key_count];
    float* timeline_key_angles = new float[timeline_key_count];
    float2* timeline_key_scales = new float2[timeline_key_count];
    int* timeline_key_z_indices = new int[timeline_key_count];
    int* timeline_key_spins = new int[timeline_key_count];
    int* timeline_mainline_first_key_map = new int[timeline_key_count];
    int* timeline_mainline_key_map = new int[timeline_key_count];
	int* timeline_symbol_map = new int[timeline_key_count];
	int* timeline_key_times = new int[timeline_key_count];
	int2* timeline_key_dimensions = new int2[timeline_key_count];
	int* timeline_key_symbol_frame_nums = new int[timeline_key_count];
	int* timeline_key_parent_ids = new int[timeline_key_count];
	float* timeline_key_alphas = new float[timeline_key_count];
	char** timeline_key_names = allocate_strings(timeline_key_count, MAX_NAME_LENGTH);	

	bone* anim_bone_frames = new bone[bone_frame_count];
	bone* anim_flattened_bone_frames = new bone[bone_frame_count];

    import_animations(
        IN  scml,
        OUT anim_names,
        OUT anim_roots,
        OUT anim_lengths,
        OUT anim_framerates,
		OUT anim_loopings
        );

    import_mainline(
        IN  scml,
        OUT mainline_key_start_indices,
        OUT mainline_key_counts,
        OUT mainline_key_positions,
        OUT mainline_key_pivots,
        OUT mainline_key_angles,
        OUT mainline_key_scales,
        OUT mainline_key_z_indices,
        OUT mainline_key_spins, 
        OUT mainline_key_ids,
        OUT mainline_key_timeline_ids,
		OUT mainline_parent_ids
        );

    import_timeline_maps(
        IN  scml,
        IN  mainline_key_start_indices,
        IN  mainline_key_counts,
        IN  mainline_key_ids,
		IN  mainline_key_timeline_ids,
		IN  symbol_frame_count,
        IN  symbol_ids,
        OUT timeline_mainline_first_key_map,
        OUT timeline_mainline_key_map,        
        OUT timeline_symbol_map
        );

    apply_symbols(
        IN  symbol_frame_pivots,
		IN  symbol_frame_dimensions,
        IN  timeline_symbol_map,
        IN  timeline_key_count,
        OUT timeline_key_pivots,
		OUT timeline_key_dimensions
        );

    apply_mainline_keys(
        IN  timeline_key_count,
        IN  timeline_mainline_first_key_map,
        IN  mainline_key_positions,
        IN  mainline_key_pivots,
        IN  mainline_key_angles,
        IN  mainline_key_scales,
        IN  mainline_key_z_indices,
        IN  mainline_key_spins,
        OUT timeline_key_positions,
        OUT timeline_key_pivots,
        OUT timeline_key_angles,
        OUT timeline_key_scales,
        OUT timeline_key_z_indices,
        OUT timeline_key_spins        
        );

    apply_mainline_keys(
		IN  timeline_key_count,
        IN  timeline_mainline_key_map,
        IN  mainline_key_z_indices,
        OUT timeline_key_z_indices
        );

	apply_mainline_parent_ids(
		timeline_key_count,
		timeline_mainline_key_map,
		mainline_parent_ids,
		timeline_key_parent_ids
		);


    import_timelines(
        IN scml,
		OUT anim_timeline_counts,
		OUT anim_timeline_start_indices,
		OUT anim_timeline_key_start_indices,
        OUT anim_timeline_key_counts,
        OUT timeline_key_positions,
        OUT timeline_key_pivots,
        OUT timeline_key_angles,
        OUT timeline_key_scales,
        OUT timeline_key_spins,
		OUT timeline_key_times,
		OUT timeline_key_symbol_frame_nums,
		OUT timeline_key_names,
		OUT timeline_key_alphas
        );

	import_bones(
		IN  scml,
		OUT anim_bone_counts,
		OUT anim_bone_start_indices,
		OUT anim_bone_key_start_indices,
		OUT anim_bone_key_counts,
		OUT anim_bone_keys
		);

    // conversion
    for(int i = 0; i < symbol_frame_count; ++i)
    {
        convert_image_path_to_name(
            IN  symbol_frame_image_paths[i],
            OUT symbol_frame_image_names[i]
            );
    }

	for(int i = 0; i < timeline_key_count; ++i)
	{
		int max_z = timeline_count;
		convert_z_index(
			IN  max_z,
			OUT timeline_key_z_indices[i]
			);

	}

	int2* timeline_frame_dimensions = new int2[element_count];
    float2* timeline_frame_positions = new float2[element_count];
    float2* timeline_frame_pivots = new float2[element_count];
    float* timeline_frame_angles = new float[element_count];
    float2* timeline_frame_scales = new float2[element_count];
    int* timeline_frame_z_indices = new int[element_count];
    int* timeline_frame_spins = new int[element_count];
	int* timeline_frame_symbol_frame_nums = new int[element_count];
	int* timeline_frame_parent_ids = new int[element_count];
	float* timeline_frame_alphas = new float[element_count];
	char** timeline_frame_names = allocate_strings(element_count, MAX_NAME_LENGTH);
	char** timeline_frame_layer_names = allocate_strings(element_count, MAX_NAME_LENGTH);
	
	int element_index = 0;
	int frame_element_count_index = 0;
    for(int i = 0; i < anim_count; ++i)
    {
        int timeline_start_index = anim_timeline_start_indices[i];
		anim_frame_start_indices[i] = frame_element_count_index;
        convert_anim_timelines_to_frames(
            anim_lengths[i],
			anim_loopings[i],
            anim_timeline_counts[i],
            &anim_timeline_key_start_indices[timeline_start_index],
            &anim_timeline_key_counts[timeline_start_index],
            timeline_key_times,
            timeline_key_positions,
			timeline_key_dimensions,
			timeline_key_pivots,
			timeline_key_scales,
			timeline_key_angles,
			timeline_key_z_indices,
			timeline_key_spins,
			timeline_key_symbol_frame_nums,
			timeline_key_names,
			timeline_key_parent_ids,
			timeline_key_alphas,
			anim_frame_counts[i],			
			&frame_element_counts[frame_element_count_index],
			&frame_element_start_indices[frame_element_count_index],
			&frame_indices[frame_element_count_index],
			&frame_positions[frame_element_count_index],
			&frame_dimensions[frame_element_count_index],
			timeline_frame_symbol_frame_nums,
			timeline_frame_names,
			timeline_frame_layer_names,
			timeline_frame_positions,
			timeline_frame_dimensions,
			timeline_frame_pivots,
			timeline_frame_scales,
			timeline_frame_angles,
			timeline_frame_z_indices,
			timeline_frame_parent_ids,
			timeline_frame_alphas,
			element_index
            );
		frame_element_count_index += anim_frame_counts[i];
    }

	// convert_bone_keys_to_frames
	int bone_frame_index = 0;
	for(int anim_index = 0; anim_index < anim_count; ++anim_index)
	{
		int anim_length = anim_lengths[anim_index];
		int bone_count = anim_bone_counts[anim_index];
		bool looping = anim_loopings[anim_index];

		int* bone_key_start_indices = &anim_bone_key_start_indices[anim_bone_start_indices[anim_index]];
		int* bone_key_counts = &anim_bone_key_counts[anim_bone_start_indices[anim_index]];
		anim_bone_frame_start_indices[anim_index] = bone_frame_index;
		for(int frame_num = 0; frame_num < anim_length; frame_num+=1000/FRAME_RATE)
		{
			for(int bone_index = 0; bone_index < bone_count; ++bone_index)
			{
				bone* bone_keys = &anim_bone_keys[bone_key_start_indices[bone_index]];
				int bone_key_count = bone_key_counts[bone_index];

				int start_key = 0;
				int end_key = 0;
				for(int bone_key_index = 0; bone_key_index < bone_key_count; ++bone_key_index)
				{
					bone& bone_key = bone_keys[bone_key_index];
					if(bone_key.time == frame_num)
					{
						start_key = bone_key_index;
						end_key = bone_key_index;
						break;
					}
					else if(bone_key.time < frame_num)
					{
						start_key = bone_key_index;
						end_key = bone_key_index;
					}
					else
					{
						end_key = bone_key_index;
						break;
					}
				}

				bone& bone_frame = anim_bone_frames[bone_frame_index++];

				int start_time = bone_keys[start_key].time;
				int end_time = bone_keys[end_key].time;

				if(start_key == bone_key_count - 1 && looping && start_time != frame_num)
				{
					end_key = 0;
					end_time = anim_length;
				}

				float blend = 0.f;
				if( end_time != start_time )
				{
					blend = ( float )( frame_num - start_time ) / ( float )( end_time - start_time );
				}

				bone& a = bone_keys[start_key];
				bone& b = bone_keys[end_key];
				bone_frame.x = lerp(a.x, b.x, blend);
				bone_frame.y = lerp(a.y, b.y, blend);
				bone_frame.scale_x = lerp(a.scale_x, b.scale_x, blend);
				bone_frame.scale_y = lerp(a.scale_y, b.scale_y, blend);
				bone_frame.id = a.id;
				bone_frame.parent_id = a.parent_id;
				bone_frame.spin = a.spin;
				bone_frame.time = frame_num;
				bone_frame.angle = lerp_angle(a.angle, b.angle, blend, a.spin);
				create_trans_rot_scale_pivot_matrix(
					IN  float2(bone_frame.x, bone_frame.y),
					IN  bone_frame.angle,
					IN  float2(bone_frame.scale_x, bone_frame.scale_y),
					IN  float2(0, 0),
					OUT bone_frame.flattened 
					);
			}
		}
	}

	// flatten bones
	for(int anim_index = 0; anim_index < anim_count; ++anim_index)
	{
		int anim_length = anim_lengths[anim_index];
		int bone_count = anim_bone_counts[anim_index];
		int frame_index = 0;
		for(int frame_num = 0; frame_num < anim_length; frame_num+=1000/FRAME_RATE)
		{
			bone* bone_frames = &anim_bone_frames[anim_bone_frame_start_indices[anim_index] + frame_index * bone_count];
			bone* flattened_bone_frames = &anim_flattened_bone_frames[anim_bone_frame_start_indices[anim_index]  + frame_index * bone_count];
			for(int bone_index = 0; bone_index < bone_count; ++bone_index)
			{
				bone& fb = flattened_bone_frames[bone_index];				
				fb = bone_frames[bone_index];	
				fb.is_flattened = false;
			}

			for(int bone_index = 0; bone_index < bone_count; ++bone_index)
			{
				flatten(flattened_bone_frames[bone_index], bone_count, flattened_bone_frames);
			}
			++frame_index;
		}
	}


	//apply bones to frames
	for(int anim_index = 0; anim_index < anim_count; ++anim_index)
	{        
		int anim_length = anim_lengths[anim_index];
        if(anim_length > 0)
        {
			int frame_start_index = anim_frame_start_indices[anim_index];

			int* anim_frame_element_counts = &frame_element_counts[frame_start_index];
            int* anim_frame_element_start_indices = &frame_element_start_indices[frame_start_index];

			int bone_count = anim_bone_counts[anim_index];
			for(int frame_index = 0; frame_index < anim_length/(1000/FRAME_RATE); ++frame_index)
			{				
				bone* bones = &anim_flattened_bone_frames[anim_bone_frame_start_indices[anim_index]  + frame_index * bone_count];
				int element_start_index = anim_frame_element_start_indices[frame_index];
				int element_count = anim_frame_element_counts[frame_index];
				matrix3* matrices = &element_matrices[element_start_index];
				int* parents = &timeline_frame_parent_ids[element_start_index];
				float2* scales = &timeline_frame_scales[element_start_index];
				float2* positions = &timeline_frame_positions[element_start_index];
				float* angles = &timeline_frame_angles[element_start_index];

				for(int element_index = 0; element_index < element_count; ++element_index)
				{
					if(parents[element_index] >= 0)
					{
						for(int bone_index = 0; bone_index < bone_count; ++bone_index)
						{
							bone& b = bones[bone_index];
							if(b.id == parents[element_index])
							{
								float2& pos = positions[element_index];
								float2& scale = scales[element_index];
								float& angle = angles[element_index];

								scale.x*=b.scale_x;
								scale.y*=b.scale_y;

								float sx = pos.x * b.scale_x;
								float sy = pos.y * b.scale_y;
								float s = sin(b.angle);
								float c = cos(b.angle);
								pos.x= b.x + sx*c - sy*s;
								pos.y= b.y + sx*s + sy*c;
								angle+=b.angle;
							}
						}
					}
				}
			}
		}
	}

    for(int i = 0; i < element_count; ++i)
    {
        convert_timeline_frame_to_element_frame(
            IN  timeline_frame_dimensions[i],
            IN  timeline_frame_pivots[i],
            IN  timeline_frame_scales[i],
            IN  timeline_frame_positions[i],
            IN  timeline_frame_angles[i],
            OUT element_matrices[i]
            );
    }

    int build_anim_index = -1;
    for(int i = 0; i < anim_count; ++i)
    {
        if(strstr(anim_names[i], "BUILD") == anim_names[i])
        {
            build_anim_index = i;
            break;
        }
    }

    if(build_anim_index >= 0)
    {
        bool character_build = strcmp(anim_names[build_anim_index], "BUILD_PLAYER") == 0;

        // apply_swappable_frame_durations
        for(int i = 0; i < symbol_frame_count; ++i)
        {
            if(character_build)
            {
                symbol_frame_durations[i] = 1;
            }
            else
            {
                symbol_frame_durations[i] = 10;
            }
            
        }
        
        // convert_symbol_frame_pivots
        for(int i = 0; i < symbol_frame_count; ++i)
        {
            convert_pivot(
                IN  symbol_frame_dimensions[i],
                IN  symbol_frame_pivots[i],
                OUT symbol_frame_pivots[i]
                );
        }

        convert_to_swappable_build(
            IN  timeline_key_count,
            IN  timeline_symbol_map,
            IN  timeline_key_times,
            IN  timeline_key_dimensions,
            IN  timeline_key_pivots,
            OUT symbol_frame_pivots
            );
    }
    else
    {
#ifdef NEWPIVOTS
        // convert_symbol_frame_pivots
        for(int i = 0; i < symbol_frame_count; ++i)
        {
            convert_pivot(
                IN  symbol_frame_dimensions[i],
                IN  symbol_frame_pivots[i],
                OUT symbol_frame_pivots[i]
                );
        }
#else
        // clear_symbol_frame_pivots
        for(int i = 0; i < symbol_frame_count; ++i)
        {
            symbol_frame_pivots[i].x = 0;
            symbol_frame_pivots[i].y = 0;
        }
#endif
    }

	build_metadata_t build_metadata;
    
    // export builds
    export_build(
		OUT build_metadata,
        IN (Path(get_asset_temp_dir())/"build.xml").c_str(),
        IN build_name,
        IN symbol_count, 
        IN symbol_names, 
        IN symbol_frame_counts, 
        IN symbol_frame_indices, 
        IN symbol_frame_nums, 
        IN symbol_frame_durations, 
        IN symbol_frame_image_names, 
        IN symbol_frame_dimensions, 
        IN symbol_frame_pivots
        );


    // export animations
    if(anim_count > 0)
    {
        xml_writer animation_writer;
        animation_writer.begin_doc( (Path(get_asset_temp_dir())/"animation.xml").c_str() );
        animation_writer.push("Anims");

        for(int i = 0; i < anim_count; ++i)
        {            
            if(anim_lengths[i] >= (1000 / FRAME_RATE))
            {
                int frame_start_index = anim_frame_start_indices[i];
                export_animation(
					IN build_metadata,
                    IN animation_writer,
                    IN anim_names[i],
                    IN anim_roots[i],
                    IN anim_lengths[i]/(1000/FRAME_RATE),
                    IN anim_framerates[i],
                    IN &frame_indices[frame_start_index],
                    IN &frame_dimensions[frame_start_index],
                    IN &frame_positions[frame_start_index],
                    IN &frame_element_counts[frame_start_index],
                    IN &frame_element_start_indices[frame_start_index],
                    IN timeline_frame_names,
                    IN timeline_frame_layer_names,
                    IN timeline_frame_symbol_frame_nums,
                    IN timeline_frame_z_indices,
					IN timeline_frame_alphas,
                    IN element_matrices
					);
            }        
        }

        animation_writer.pop();
        animation_writer.end_doc();
    }

	image_paths = symbol_frame_image_paths;
	image_path_count = symbol_frame_count;
}

int main( int argument_count, char** arguments )
{    
    set_application_folder( arguments[0] );
    begin_log();

	bool force = extract_arg(argument_count, arguments, "f");
	bool ignore_self_date = extract_arg(argument_count, arguments, "ignore-self-date");

	if( argument_count != 3 )
	{
		error( "ERROR: Invalid number of arguments!\n" );
	}
    
	Path input_file_path = arguments[1];
	input_file_path.makeAbsolute();

	if( !input_file_path.exists() )
	{
		error( "ERROR: Could not open '%s'!\n", input_file_path.c_str() );
	}
	//printf("input file: %s\n", input_file_path.basename().c_str());

	Path asset_name = input_file_path.basename();
	asset_name.removeExtension();
	set_asset_name( asset_name.c_str() );

	Path input_folder = input_file_path.dirname();

	Path output_package_file_path = input_file_path;
	output_package_file_path.replaceExtension("zip");

	Path output_dir = arguments[2];
	output_dir.makeAbsolute();

	Path built_package_path = output_dir/"anim"/output_package_file_path.basename();

    SCML::Data scml( input_file_path.c_str() );	

	char** image_paths = 0;
	int image_path_count;

	build_scml(scml, image_paths, image_path_count);

	bool up_to_date = false;
	/*
	 * Existence checks are implicit.
	 * If neither compared files of a pair exist, the check fails, since the inequality is strict.
	 */
    if(	!force
		&& input_file_path.isOlderThan(built_package_path)
        && (ignore_self_date || Path(arguments[0]).isOlderThan(built_package_path))
        && built_package_path.isNewerThan(output_package_file_path)
	  ){
		up_to_date = true;
	}

	Path image_list_path = Path(get_asset_temp_dir())/"images.lst";
    FILE* image_list_file = fopen(image_list_path.c_str(), "w");
    for(int i = 0; i < image_path_count; ++i)
    {
		Path path = input_folder/image_paths[i];
        if(!path.exists())
        {
            error("ERROR: Missing image '%s' referenced by '%s'.\n", image_paths[i], input_file_path.basename().c_str());
        }
		else if(path.isNewerThan(built_package_path)) {
			up_to_date = false;
		}
        fprintf(image_list_file, "%s\n", path.c_str());
    }
    fclose(image_list_file);

    if(up_to_date){
		log_and_fprint(stderr, "%s is up to date.\n", built_package_path.c_str());
        return 0;
    }

	Path app_folder(get_application_folder());

	char command_line[32768];
	sprintf(command_line,
			"%s \"%s\" \"%s\" \"%s\"",
			get_python(),
			(app_folder/"compiler_scripts"/"zipanim.py").c_str(),
			get_asset_temp_dir(),
			output_package_file_path.c_str()
	);
	run( command_line, true, "Packaging '%s'", output_package_file_path.basename().c_str() );

	sprintf(command_line,
			"%s \"%s\" --skip_update_prefabs --outputdir \"%s\" --prefabsdir \"%s\" \"%s\"",
			get_python(),
			(app_folder/"exported"/"export.py").c_str(),
			output_dir.c_str(),
			(app_folder/"data").c_str(),
			output_package_file_path.c_str()
	);
    run( command_line, true, "Building '%s'", output_package_file_path.basename().c_str() );

    end_log();

	return 0;
}
