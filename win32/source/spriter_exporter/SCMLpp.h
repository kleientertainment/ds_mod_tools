#ifndef _SCMLPP_H__
#define _SCMLPP_H__


// Compile-time switches for container classes
// Do you want the STL to be used?
#ifndef SCML_NO_STL
    #include <string>
    #include <map>
    #include <vector>

    #define SCML_STRING std::string
    #define SCML_MAP(a,b) std::map< a, b >
    #define SCML_VECTOR(a) std::vector< a >
    #define SCML_PAIR(a,b) std::pair< a, b >
    
    #define SCML_VECTOR_SIZE(v) (v).size()
    #define SCML_VECTOR_RESIZE(v,size) (v).resize(size)
    #define SCML_VECTOR_CLEAR(v) (v).clear()
    
    #define SCML_PAIR_FIRST(p) (p).first
    #define SCML_PAIR_SECOND(p) (p).second
    
    template<typename A, typename B>
    inline SCML_PAIR(A, B) SCML_MAKE_PAIR(A const& a, B const& b)
    {
        return std::make_pair(a, b);
    }
    
    #define SCML_MAP_SIZE(m) (m).size()
    #define SCML_MAP_INSERT(m,a,b) m.insert(std::make_pair((a),(b))).second
    
    // Be careful with these...  Macros don't nest well.  Use a typedef as necessary for the map parameters.
    #define SCML_BEGIN_MAP_FOREACH(m,a,b,name) for(SCML_MAP(a , b)::iterator _iter_e = m.begin(); _iter_e != m.end(); _iter_e++) { b& name = _iter_e->second;
    #define SCML_END_MAP_FOREACH }
    #define SCML_BEGIN_MAP_FOREACH_CONST(m,a,b,name) for(SCML_MAP(a , b)::const_iterator _iter_e = m.begin(); _iter_e != m.end(); _iter_e++) { b const& name = _iter_e->second;
    #define SCML_END_MAP_FOREACH_CONST }
    
    #define SCML_TO_CSTRING(s) (s).c_str()
    #define SCML_STRING_SIZE(s) (s).size()
    #define SCML_SET_STRING(s,value) s = value
    #define SCML_STRING_APPEND(s,value) s += value
    
    template<typename A, typename B>
    inline B SCML_MAP_FIND(SCML_MAP(A, B) const& m, A const& key)
    {
        typename SCML_MAP(A,B)::const_iterator e = m.find(key);
        if(e == m.end())
            return B();
        return e->second;
    }
#else

    // TODO: Provide an alternative to the STL for string, map, and vector.

#endif

#include "tinyxml.h"

/*! \brief Namespace for SCMLpp
 */
namespace SCML
{

/*! \brief Representation and storage of an SCML file in memory.
 *
 *
 */
class Data
{
public:

    SCML_STRING name;
    SCML_STRING scml_version;
    SCML_STRING generator;
    SCML_STRING generator_version;
    bool pixel_art_mode;

    class Folder;
    SCML_MAP(int, Folder*) folders;
    class Atlas;
    SCML_MAP(int, Atlas*) atlases;
    class Entity;
    SCML_MAP(int, Entity*) entities;
    class Character_Map;
    SCML_MAP(int, Character_Map*) character_maps;

    Data();
    Data(const SCML_STRING& file);
    Data(TiXmlElement* elem);
    Data(const Data& copy);
    Data& operator=(const Data& copy);
    ~Data();

    bool load(const SCML_STRING& file);
    bool load(TiXmlElement* elem);
    Data& clone(const Data& copy, bool skip_base = false);
    void log(int recursive_depth = 0) const;
    void clear();



    class Meta_Data
    {
    public:

        class Variable;
        SCML_MAP(SCML_STRING, Variable*) variables;
        class Tag;
        SCML_MAP(SCML_STRING, Tag*) tags;

        Meta_Data();
        Meta_Data(TiXmlElement* elem);

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        class Variable
        {
        public:

            SCML_STRING name;
            SCML_STRING type;

            SCML_STRING value_string;
            int value_int;
            float value_float;

            Variable();
            Variable(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();
        };

        class Tag
        {
        public:

            SCML_STRING name;

            Tag();
            Tag(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();
        };
    };


    class Meta_Data_Tweenable
    {
    public:

        class Variable;
        SCML_MAP(SCML_STRING, Variable*) variables;
        class Tag;
        SCML_MAP(SCML_STRING, Tag*) tags;

        Meta_Data_Tweenable();
        Meta_Data_Tweenable(TiXmlElement* elem);

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        class Variable
        {
        public:

            SCML_STRING name;
            SCML_STRING type;
            SCML_STRING value_string;
            int value_int;
            float value_float;
            SCML_STRING curve_type;
            float c1;
            float c2;

            Variable();
            Variable(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();

        };

        class Tag
        {
        public:

            SCML_STRING name;

            Tag();
            Tag(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();
        };
    };

    Meta_Data* meta_data;

    class Folder
    {
    public:

        int id;
        SCML_STRING name;

        class File;
        SCML_MAP(int, File*) files;

        Folder();
        Folder(TiXmlElement* elem);

        ~Folder();

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        class File
        {
        public:

            SCML_STRING type;
            int id;
            SCML_STRING name;
            float pivot_x;
            float pivot_y;
            int width;
            int height;
            int atlas_x;
            int atlas_y;
            int offset_x;
            int offset_y;
            int original_width;
            int original_height;

            File();
            File(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();

        };
    };

    class Atlas
    {
    public:
        int id;
        SCML_STRING data_path;
        SCML_STRING image_path;

        class Folder;
        SCML_MAP(int, Folder*) folders;

        Atlas();
        Atlas(TiXmlElement* elem);

        ~Atlas();

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        class Folder
        {
        public:

            int id;
            SCML_STRING name;

            class Image;
            SCML_MAP(int, Image*) images;

            Folder();
            Folder(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();


            class Image
            {
            public:

                int id;
                SCML_STRING full_path;

                Image();
                Image(TiXmlElement* elem);

                bool load(TiXmlElement* elem);
                void log(int recursive_depth = 0) const;
                void clear();

            };
        };
    };

    class Entity
    {
    public:

        int id;
        SCML_STRING name;

        class Animation;
        SCML_MAP(int, Animation*) animations;

        Entity();
        Entity(TiXmlElement* elem);

        ~Entity();

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        Meta_Data* meta_data;

        class Animation
        {
        public:

            int id;
            SCML_STRING name;
            int length;
            SCML_STRING looping;
            int loop_to;

            Meta_Data* meta_data;

            // More to follow...
            class Mainline
            {
            public:

                Mainline();
                Mainline(TiXmlElement* elem);

                ~Mainline();

                bool load(TiXmlElement* elem);
                void log(int recursive_depth = 0) const;
                void clear();

                class Key;
                SCML_MAP(int, Key*) keys;

                class Key
                {
                public:

                    int id;
                    int time;

                    Meta_Data* meta_data;

                    Key();
                    Key(TiXmlElement* elem);

                    ~Key();

                    bool load(TiXmlElement* elem);
                    void log(int recursive_depth = 0) const;
                    void clear();


                    class Object;
                    class Object_Ref;

                    class Object_Container
                    {
                    public:
                        Object* object;
                        Object_Ref* object_ref;

                        Object_Container()
                            : object(NULL), object_ref(NULL)
                        {}
                        Object_Container(Object* object)
                            : object(object), object_ref(NULL)
                        {}
                        Object_Container(Object_Ref* object_ref)
                            : object(NULL), object_ref(object_ref)
                        {}

                        bool hasObject() const
                        {
                            return (object != NULL);
                        }
                        bool hasObject_Ref() const
                        {
                            return (object_ref != NULL);
                        }
                    };

                    SCML_MAP(int, Object_Container) objects;


                    class Bone;
                    class Bone_Ref;

                    class Bone_Container
                    {
                    public:
                        Bone* bone;
                        Bone_Ref* bone_ref;

                        Bone_Container()
                            : bone(NULL), bone_ref(NULL)
                        {}
                        Bone_Container(Bone* bone)
                            : bone(bone), bone_ref(NULL)
                        {}
                        Bone_Container(Bone_Ref* bone_ref)
                            : bone(NULL), bone_ref(bone_ref)
                        {}

                        bool hasBone() const
                        {
                            return (bone != NULL);
                        }
                        bool hasBone_Ref() const
                        {
                            return (bone_ref != NULL);
                        }
                    };

                    SCML_MAP(int, Bone_Container) bones;

                    class Bone
                    {
                    public:

                        int id;
                        int parent;  // a bone id
                        float x;
                        float y;
                        float angle;
                        float scale_x;
                        float scale_y;
                        float r;
                        float g;
                        float b;
                        float a;
                        Meta_Data* meta_data;


                        Bone();
                        Bone(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();

                    };

                    class Bone_Ref
                    {
                    public:

                        int id;
                        int parent;  // a bone id
                        int timeline;
                        int key;

                        Bone_Ref();
                        Bone_Ref(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();
                    };

                    class Object
                    {
                    public:

                        int id;
                        int parent; // a bone id
                        SCML_STRING object_type;
                        int atlas;
                        int folder;
                        int file;
                        SCML_STRING usage;
                        SCML_STRING blend_mode;
                        SCML_STRING name;
                        float x;
                        float y;
                        float pivot_x;
                        float pivot_y;
                        int pixel_art_mode_x;
                        int pixel_art_mode_y;
                        int pixel_art_mode_pivot_x;
                        int pixel_art_mode_pivot_y;
                        float angle;
                        float w;
                        float h;
                        float scale_x;
                        float scale_y;
                        float r;
                        float g;
                        float b;
                        float a;
                        SCML_STRING variable_type;
                        SCML_STRING value_string;
                        int value_int;
                        int min_int;
                        int max_int;
                        float value_float;
                        float min_float;
                        float max_float;
                        int animation;
                        float t;
                        int z_index;
                        float volume;
                        float panning;

                        Meta_Data* meta_data;

                        Object();
                        Object(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();

                    };

                    class Object_Ref
                    {
                    public:

                        int id;
                        int parent;  // a bone id
                        int timeline;
                        int key;
                        int z_index;
                        float abs_x;
                        float abs_y;
                        float abs_pivot_x;
                        float abs_pivot_y;
                        bool found_pivot_x;
                        bool found_pivot_y;
                        float abs_angle;
                        float abs_scale_x;
                        float abs_scale_y;
                        float abs_a;
                        int folder;
                        int file;

                        Object_Ref();
                        Object_Ref(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();
                    };
                };

            };

            Mainline mainline;

            class Timeline;
            SCML_MAP(int, Timeline*) timelines;

            Animation();
            Animation(TiXmlElement* elem);

            ~Animation();

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();



            class Timeline
            {
            public:

                int id;
                SCML_STRING name;
                SCML_STRING object_type;
                SCML_STRING variable_type;
                SCML_STRING usage;
                Meta_Data* meta_data;

                Timeline();
                Timeline(TiXmlElement* elem);

                ~Timeline();

                bool load(TiXmlElement* elem);
                void log(int recursive_depth = 0) const;
                void clear();

                class Key;
                SCML_MAP(int, Key*) keys;

                class Key
                {
                public:

                    int id;
                    int time;
                    SCML_STRING curve_type;
                    float c1;
                    float c2;
                    int spin;

                    bool has_object;

                    Key();
                    Key(TiXmlElement* elem);

                    ~Key();

                    bool load(TiXmlElement* elem);
                    void log(int recursive_depth = 0) const;
                    void clear();


                    Meta_Data_Tweenable* meta_data;

                    class Bone
                    {
                    public:

                        float x;                        
                        float y;
                        float angle;
                        float scale_x;
                        float scale_y;
                        float r;
                        float g;
                        float b;
                        float a;
                        Meta_Data_Tweenable* meta_data;

                        Bone();
                        Bone(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();
                    };

                    Bone bone;

                    class Object
                    {
                    public:

                        //SCML_STRING object_type; // Does this exist?
                        int atlas;
                        int folder;
                        int file;
                        //SCML_STRING usage;  // Does this exist?
                        SCML_STRING name;
                        float x;
                        float y;
                        float pivot_x;
                        float pivot_y;
                        // pixel_art_mode stuff?
                        float angle;
                        float w;
                        float h;
                        float scale_x;
                        float scale_y;
                        float r;
                        float g;
                        float b;
                        float a;
                        SCML_STRING blend_mode;
                        //SCML_STRING variable_type; // Does this exist?
                        SCML_STRING value_string;
                        int value_int;
                        int min_int;
                        int max_int;
                        float value_float;
                        float min_float;
                        float max_float;
                        int animation;
                        float t;
                        //int z_index; // Does this exist?  Object_Ref has it, so probably not.
                        float volume;
                        float panning;
                        Meta_Data_Tweenable* meta_data;

                        bool found_x;
                        bool found_y;
                        bool found_pivot_x;
                        bool found_pivot_y;
                        bool found_angle;
                        bool found_scale_x;
                        bool found_scale_y;

                        Object();
                        Object(TiXmlElement* elem);

                        bool load(TiXmlElement* elem);
                        void log(int recursive_depth = 0) const;
                        void clear();

                    };

                    Object object;
                };
            };
        };
    };

    class Character_Map
    {
    public:

        int id;
        SCML_STRING name;

        Character_Map();
        Character_Map(TiXmlElement* elem);

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

        class Map
        {
        public:

            int atlas;
            int folder;
            int file;
            int target_atlas;
            int target_folder;
            int target_file;

            Map();
            Map(TiXmlElement* elem);

            bool load(TiXmlElement* elem);
            void log(int recursive_depth = 0) const;
            void clear();
        };

        Map map;
    };

    class Document_Info
    {
    public:

        SCML_STRING author;
        SCML_STRING copyright;
        SCML_STRING license;
        SCML_STRING version;
        SCML_STRING last_modified;
        SCML_STRING notes;

        Document_Info();
        Document_Info(TiXmlElement* elem);

        bool load(TiXmlElement* elem);
        void log(int recursive_depth = 0) const;
        void clear();

    };

    Document_Info document_info;

    int getNumAnimations(int entity) const;
};

/*! \brief A storage class for images in a renderer-specific format (to be inherited).
 */
class FileSystem
{
public:

    virtual ~FileSystem() {}

    /*! \brief Loads all images referenced by the given SCML data.
     * \param data SCML data object
     */
    virtual void load(SCML::Data* data);

    /*! \brief Loads an image from a file and stores it so that the folderID and fileID can be used to reference the image.
     * \param folderID Integer folder ID
     * \param fileID Integer file ID
     * \param filename Path of the image file
     * \return true on success, false on failure
     */
    virtual bool loadImageFile(int folderID, int fileID, const SCML_STRING& filename) = 0;

    /*! \brief Cleans up all memory used by the FileSystem to store images, resetting it to an empty state.
     */
    virtual void clear() = 0;

    /*! \brief Gets the dimensions of an image
     * \param folderID Integer folder ID
     * \param fileID Integer file ID
     * \return A pair consisting of the width and height of the image.  Returns (0,0) on error.
     */
    virtual SCML_PAIR(unsigned int, unsigned int) getImageDimensions(int folderID, int fileID) const = 0;
};

}


#endif
