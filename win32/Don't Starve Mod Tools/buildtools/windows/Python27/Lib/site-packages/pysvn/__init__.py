'''
 ====================================================================
 Copyright (c) 2003-2009 Barry A Scott.  All rights reserved.

 This software is licensed as described in the file LICENSE.txt,
 which you should have received as part of this distribution.

 ====================================================================
'''
import sys
try:
    import UserDict
    user_dict_base = UserDict.IterableUserDict

except ImportError:
    import collections
    user_dict_base = collections.UserDict

class PysvnDictBase(user_dict_base):
    def __init__( self, value_dict, name='' ):
        user_dict_base.__init__( self, value_dict )
        self.__name = name
        if self.__name is None:
            print( '%s given None as name' % self.__class__.__name__ )

    def __getattr__( self, name ):
        if name in self.data:
            return self.data[ name ]
        raise AttributeError( "%s instance has no attribute '%s'" % (self.__class__.__name__, name) )

    def __repr__( self ):
        return '<%s %s>' % (self.__class__.__name__, repr(self.__name))


class PysvnDirent(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict, value_dict.get( 'name', None ) )

class PysvnList(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict, value_dict.get( 'path', None ) )

class PysvnEntry(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict, value_dict.get( 'name', None ) )

class PysvnInfo(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict )

class PysvnLock(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict )

class PysvnLog(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict )

class PysvnLogChangedPath(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict )

class PysvnWcInfo(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict )

class PysvnStatus(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict, value_dict.get( 'path', None ) )

class PysvnDiffSummary(PysvnDictBase):
    def __init__( self, value_dict ):
        PysvnDictBase.__init__( self, value_dict, value_dict.get( 'path', None ) )

# An indication that you are interested in the @c kind field
SVN_DIRENT_KIND        = 0x00001

# An indication that you are interested in the @c size field
SVN_DIRENT_SIZE        = 0x00002

# An indication that you are interested in the @c has_props field
SVN_DIRENT_HAS_PROPS   = 0x00004

# An indication that you are interested in the @c created_rev field
SVN_DIRENT_CREATED_REV = 0x00008

# An indication that you are interested in the @c time field
SVN_DIRENT_TIME        = 0x00010

# An indication that you are interested in the @c last_author field
SVN_DIRENT_LAST_AUTHOR = 0x00020

# A combination of all the dirent fields
SVN_DIRENT_ALL         = 0xffffffff

try:
    maj_min = sys.version_info[:2]

    import _pysvn_2_7
    _pysvn = _pysvn_2_7

    for key, value in _pysvn.__dict__.items():
        if not key.startswith( '__' ):
            globals()[ key ] = value

except ImportError as e:
    # check for common installation errors that show up as ImportError
    if ': undefined symbol:' in str(e):
        raise ImportError( 'pysvn was built against newer (svn, apr, etc.) libraries then the ones installed on this system. %s' % str(e) )
    else:
        raise

def Client( config_dir='' ):
    return _pysvn._Client( config_dir, result_wrappers=globals() )

def Transaction( repos_path, transaction_name, is_revision=False ):
    return _pysvn._Transaction( repos_path, transaction_name, is_revision, result_wrappers=globals() )

class svn_err:
    bad_containing_pool = 125000
    bad_filename = 125001
    bad_url = 125002
    bad_date = 125003
    bad_mime_type = 125004
    bad_property_value = 125005
    bad_version_file_format = 125006
    bad_relative_path = 125007
    bad_uuid = 125008
    bad_config_value = 125009
    bad_server_specification = 125010
    bad_checksum_kind = 125011
    bad_checksum_parse = 125012
    bad_token = 125013
    bad_changelist_name = 125014
    xml_attrib_not_found = 130000
    xml_missing_ancestry = 130001
    xml_unknown_encoding = 130002
    xml_malformed = 130003
    xml_unescapable_data = 130004
    io_inconsistent_eol = 135000
    io_unknown_eol = 135001
    io_corrupt_eol = 135002
    io_unique_names_exhausted = 135003
    io_pipe_frame_error = 135004
    io_pipe_read_error = 135005
    io_write_error = 135006
    io_pipe_write_error = 135007
    stream_unexpected_eof = 140000
    stream_malformed_data = 140001
    stream_unrecognized_data = 140002
    stream_seek_not_supported = 140003
    node_unknown_kind = 145000
    node_unexpected_kind = 145001
    entry_not_found = 150000
    entry_exists = 150002
    entry_missing_revision = 150003
    entry_missing_url = 150004
    entry_attribute_invalid = 150005
    entry_forbidden = 150006
    wc_obstructed_update = 155000
    wc_unwind_mismatch = 155001
    wc_unwind_empty = 155002
    wc_unwind_not_empty = 155003
    wc_locked = 155004
    wc_not_locked = 155005
    wc_invalid_lock = 155006
    wc_not_working_copy = 155007
    wc_not_directory = 155007
    wc_not_file = 155008
    wc_bad_adm_log = 155009
    wc_path_not_found = 155010
    wc_not_up_to_date = 155011
    wc_left_local_mod = 155012
    wc_schedule_conflict = 155013
    wc_path_found = 155014
    wc_found_conflict = 155015
    wc_corrupt = 155016
    wc_corrupt_text_base = 155017
    wc_node_kind_change = 155018
    wc_invalid_op_on_cwd = 155019
    wc_bad_adm_log_start = 155020
    wc_unsupported_format = 155021
    wc_bad_path = 155022
    wc_invalid_schedule = 155023
    wc_invalid_relocation = 155024
    wc_invalid_switch = 155025
    wc_mismatched_changelist = 155026
    wc_conflict_resolver_failure = 155027
    wc_copyfrom_path_not_found = 155028
    wc_changelist_move = 155029
    wc_cannot_delete_file_external = 155030
    wc_cannot_move_file_external = 155031
    wc_db_error = 155032
    wc_missing = 155033
    wc_not_symlink = 155034
    wc_path_unexpected_status = 155035
    wc_upgrade_required = 155036
    wc_cleanup_required = 155037
    wc_invalid_operation_depth = 155038
    wc_path_access_denied = 155039
    fs_general = 160000
    fs_cleanup = 160001
    fs_already_open = 160002
    fs_not_open = 160003
    fs_corrupt = 160004
    fs_path_syntax = 160005
    fs_no_such_revision = 160006
    fs_no_such_transaction = 160007
    fs_no_such_entry = 160008
    fs_no_such_representation = 160009
    fs_no_such_string = 160010
    fs_no_such_copy = 160011
    fs_transaction_not_mutable = 160012
    fs_not_found = 160013
    fs_id_not_found = 160014
    fs_not_id = 160015
    fs_not_directory = 160016
    fs_not_file = 160017
    fs_not_single_path_component = 160018
    fs_not_mutable = 160019
    fs_already_exists = 160020
    fs_root_dir = 160021
    fs_not_txn_root = 160022
    fs_not_revision_root = 160023
    fs_conflict = 160024
    fs_rep_changed = 160025
    fs_rep_not_mutable = 160026
    fs_malformed_skel = 160027
    fs_txn_out_of_date = 160028
    fs_berkeley_db = 160029
    fs_berkeley_db_deadlock = 160030
    fs_transaction_dead = 160031
    fs_transaction_not_dead = 160032
    fs_unknown_fs_type = 160033
    fs_no_user = 160034
    fs_path_already_locked = 160035
    fs_path_not_locked = 160036
    fs_bad_lock_token = 160037
    fs_no_lock_token = 160038
    fs_lock_owner_mismatch = 160039
    fs_no_such_lock = 160040
    fs_lock_expired = 160041
    fs_out_of_date = 160042
    fs_unsupported_format = 160043
    fs_rep_being_written = 160044
    fs_txn_name_too_long = 160045
    fs_no_such_node_origin = 160046
    fs_unsupported_upgrade = 160047
    fs_no_such_checksum_rep = 160048
    fs_prop_basevalue_mismatch = 160049
    repos_locked = 165000
    repos_hook_failure = 165001
    repos_bad_args = 165002
    repos_no_data_for_report = 165003
    repos_bad_revision_report = 165004
    repos_unsupported_version = 165005
    repos_disabled_feature = 165006
    repos_post_commit_hook_failed = 165007
    repos_post_lock_hook_failed = 165008
    repos_post_unlock_hook_failed = 165009
    repos_unsupported_upgrade = 165010
    ra_illegal_url = 170000
    ra_not_authorized = 170001
    ra_unknown_auth = 170002
    ra_not_implemented = 170003
    ra_out_of_date = 170004
    ra_no_repos_uuid = 170005
    ra_unsupported_abi_version = 170006
    ra_not_locked = 170007
    ra_partial_replay_not_supported = 170008
    ra_uuid_mismatch = 170009
    ra_repos_root_url_mismatch = 170010
    ra_session_url_mismatch = 170011
    ra_dav_sock_init = 175000
    ra_dav_creating_request = 175001
    ra_dav_request_failed = 175002
    ra_dav_options_req_failed = 175003
    ra_dav_props_not_found = 175004
    ra_dav_already_exists = 175005
    ra_dav_invalid_config_value = 175006
    ra_dav_path_not_found = 175007
    ra_dav_proppatch_failed = 175008
    ra_dav_malformed_data = 175009
    ra_dav_response_header_badness = 175010
    ra_dav_relocated = 175011
    ra_dav_conn_timeout = 175012
    ra_dav_forbidden = 175013
    ra_local_repos_not_found = 180000
    ra_local_repos_open_failed = 180001
    ra_svn_cmd_err = 210000
    ra_svn_unknown_cmd = 210001
    ra_svn_connection_closed = 210002
    ra_svn_io_error = 210003
    ra_svn_malformed_data = 210004
    ra_svn_repos_not_found = 210005
    ra_svn_bad_version = 210006
    ra_svn_no_mechanisms = 210007
    ra_svn_edit_aborted = 210008
    ra_serf_sspi_initialisation_failed = 230000
    ra_serf_ssl_cert_untrusted = 230001
    ra_serf_gssapi_initialisation_failed = 230002
    ra_serf_wrapped_error = 230003
    authn_creds_unavailable = 215000
    authn_no_provider = 215001
    authn_providers_exhausted = 215002
    authn_creds_not_saved = 215003
    authn_failed = 215004
    authz_root_unreadable = 220000
    authz_unreadable = 220001
    authz_partially_readable = 220002
    authz_invalid_config = 220003
    authz_unwritable = 220004
    svndiff_invalid_header = 185000
    svndiff_corrupt_window = 185001
    svndiff_backward_view = 185002
    svndiff_invalid_ops = 185003
    svndiff_unexpected_end = 185004
    svndiff_invalid_compressed_data = 185005
    diff_datasource_modified = 225000
    apmod_missing_path_to_fs = 190000
    apmod_malformed_uri = 190001
    apmod_activity_not_found = 190002
    apmod_bad_baseline = 190003
    apmod_connection_aborted = 190004
    client_versioned_path_required = 195000
    client_ra_access_required = 195001
    client_bad_revision = 195002
    client_duplicate_commit_url = 195003
    client_is_binary_file = 195004
    client_invalid_externals_description = 195005
    client_modified = 195006
    client_is_directory = 195007
    client_revision_range = 195008
    client_invalid_relocation = 195009
    client_revision_author_contains_newline = 195010
    client_property_name = 195011
    client_unrelated_resources = 195012
    client_missing_lock_token = 195013
    client_multiple_sources_disallowed = 195014
    client_no_versioned_parent = 195015
    client_not_ready_to_merge = 195016
    client_file_external_overwrite_versioned = 195017
    client_patch_bad_strip_count = 195018
    client_cycle_detected = 195019
    client_merge_update_required = 195020
    client_invalid_mergeinfo_no_mergetracking = 195021
    client_no_lock_token = 195022
    client_forbidden_by_server = 195023
    base = 200000
    plugin_load_failure = 200001
    malformed_file = 200002
    incomplete_data = 200003
    incorrect_params = 200004
    unversioned_resource = 200005
    test_failed = 200006
    unsupported_feature = 200007
    bad_prop_kind = 200008
    illegal_target = 200009
    delta_md5_checksum_absent = 200010
    dir_not_empty = 200011
    external_program = 200012
    swig_py_exception_set = 200013
    checksum_mismatch = 200014
    cancelled = 200015
    invalid_diff_option = 200016
    property_not_found = 200017
    no_auth_file_path = 200018
    version_mismatch = 200019
    mergeinfo_parse_error = 200020
    cease_invocation = 200021
    revnum_parse_failure = 200022
    iter_break = 200023
    unknown_changelist = 200024
    reserved_filename_specified = 200025
    unknown_capability = 200026
    test_skipped = 200027
    no_apr_memcache = 200028
    atomic_init_failure = 200029
    sqlite_error = 200030
    sqlite_readonly = 200031
    sqlite_unsupported_schema = 200032
    sqlite_busy = 200033
    sqlite_resetting_for_rollback = 200034
    sqlite_constraint = 200035
    cl_arg_parsing_error = 205000
    cl_insufficient_args = 205001
    cl_mutually_exclusive_args = 205002
    cl_adm_dir_reserved = 205003
    cl_log_message_is_versioned_file = 205004
    cl_log_message_is_pathname = 205005
    cl_commit_in_added_dir = 205006
    cl_no_external_editor = 205007
    cl_bad_log_message = 205008
    cl_unnecessary_log_message = 205009
    cl_no_external_merge_tool = 205010
    cl_error_processing_externals = 205011
    assertion_fail = 235000
    assertion_only_tracing_links = 235001
