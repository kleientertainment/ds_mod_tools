import os
import sys
import time
import sublime
import imp

application_command_classes = []
window_command_classes = []
text_command_classes = []

all_command_classes = [application_command_classes, window_command_classes, text_command_classes]

all_callbacks = {'on_new': [], 'on_clone': [], 'on_load': [], 'on_close': [],
    'on_pre_save': [], 'on_post_save': [], 'on_modified': [],
    'on_selection_modified': [],'on_activated': [], 'on_deactivated': [],
    'on_project_load': [], 'on_project_close': [], 'on_query_context': [],
    'on_query_completions': []}

def unload_module(module):
    if "unload_handler" in module.__dict__:
        module.unload_handler()

    # Unload the old plugins
    if "plugins" in module.__dict__:
        for p in module.plugins:
            for cmd_cls_list in all_command_classes:
                try:
                    cmd_cls_list.remove(p)
                except ValueError:
                    pass
            for c in all_callbacks.values():
                try:
                    c.remove(p)
                except ValueError:
                    pass

def unload_plugin(fname):
    print "Unloading plugin", fname

    modulename, ext = os.path.splitext(os.path.basename(fname))

    was_loaded = modulename in sys.modules
    if was_loaded:
        m = __import__(modulename)
        unload_module(m)

def reload_plugin(fname):
    print "Reloading plugin", fname
    path = os.path.dirname(fname)

    # Change the current directory to that of the module. It's not safe to just
    # add the modules directory to sys.path, as that won't accept unicode paths
    # on Windows
    oldpath = os.getcwdu()
    os.chdir(path)

    modulename, ext = os.path.splitext(os.path.basename(fname))

    if modulename in sys.modules:
        unload_module(sys.modules[modulename])
    m_info = imp.find_module(modulename, ["."])
    m = imp.load_module(modulename, *m_info)

    # Restore the current directory
    os.chdir(oldpath)

    module_plugins = []
    for type_name in dir(m):
        try:
            t = m.__dict__[type_name]
            if t.__bases__:
                is_plugin = False
                if issubclass(t, ApplicationCommand):
                    application_command_classes.append(t)
                    is_plugin = True
                if issubclass(t, WindowCommand):
                    window_command_classes.append(t)
                    is_plugin = True
                if issubclass(t, TextCommand):
                    text_command_classes.append(t)
                    is_plugin = True

                if is_plugin:
                    module_plugins.append(t)

                if issubclass(t, EventListener):
                    obj = t()
                    for p in all_callbacks.iteritems():
                        if p[0] in dir(obj):
                            p[1].append(obj)

                    module_plugins.append(obj)

        except AttributeError:
            pass

    if len(module_plugins) > 0:
        m.plugins = module_plugins

def create_application_commands():
    cmds = []
    for class_ in application_command_classes:
        cmds.append(class_())
    return cmds

def create_window_commands(window):
    cmds = []
    for class_ in window_command_classes:
        cmds.append(class_(window))
    return cmds

def create_text_commands(view):
    cmds = []
    for class_ in text_command_classes:
        cmds.append(class_(view))
    return cmds

EVENT_TIMEOUT = 0.2
FAST_EVENT_TIMEOUT = 1 / 60.0

first_time_msgs = set()
msgs = set()

def show_timeout(plugin_name, elapsed, callback):
    global first_time_msgs
    global msgs

    key = plugin_name + callback
    msg = ("A plugin (%s) may be making Sublime Text unresponsive by taking too " +
        "long (%fs) in its %s callback.\n\nThis message can be disabled via the " +
        "detect_slow_plugins setting") % (plugin_name, elapsed, callback)

    # Give plugins one chance to respond slowly, to handle any initialisation issues etc.
    # This allowance may be removed in the future due to startup time concerns
    if not key in first_time_msgs:
        first_time_msgs.add(key)
        return

    if not key in msgs:
        msgs.add(key)
        if sublime.load_settings('Preferences.sublime-settings').get('detect_slow_plugins', True):
            sublime.error_message(msg)

blocking_api_call_count = 0
def on_blocking_api_call():
    global blocking_api_call_count
    blocking_api_call_count += 1

def run_timed_function(f, name, event_name, timeout):
    global blocking_api_call_count

    t0 = time.time()
    blocking_count = blocking_api_call_count
    ret = f()
    elapsed = time.time() - t0

    if elapsed > timeout and blocking_api_call_count == blocking_count:
        show_timeout(name, elapsed, event_name)

    return ret

def on_new(v):
    for callback in all_callbacks['on_new']:
        run_timed_function(lambda: callback.on_new(v),
            callback.__module__, "on_new", EVENT_TIMEOUT)

def on_clone(v):
    for callback in all_callbacks['on_clone']:
        run_timed_function(lambda: callback.on_clone(v),
            callback.__module__, "on_clone", EVENT_TIMEOUT)

def on_load(v):
    for callback in all_callbacks['on_load']:
        run_timed_function(lambda: callback.on_load(v),
            callback.__module__, "on_load", EVENT_TIMEOUT)

def on_close(v):
    for callback in all_callbacks['on_close']:
        run_timed_function(lambda: callback.on_close(v),
            callback.__module__, "on_close", EVENT_TIMEOUT)

def on_pre_save(v):
    for callback in all_callbacks['on_pre_save']:
        run_timed_function(lambda: callback.on_pre_save(v),
            callback.__module__, "on_pre_save", EVENT_TIMEOUT)

def on_post_save(v):
    for callback in all_callbacks['on_post_save']:
        run_timed_function(lambda: callback.on_post_save(v),
            callback.__module__, "on_post_save", EVENT_TIMEOUT)

def on_modified(v):
    for callback in all_callbacks['on_modified']:
        run_timed_function(lambda: callback.on_modified(v),
            callback.__module__, "on_modified", FAST_EVENT_TIMEOUT)

def on_selection_modified(v):
    for callback in all_callbacks['on_selection_modified']:
        run_timed_function(lambda: callback.on_selection_modified(v),
            callback.__module__, "on_selection_modified", FAST_EVENT_TIMEOUT)

def on_activated(v):
    for callback in all_callbacks['on_activated']:
        run_timed_function(lambda: callback.on_activated(v),
            callback.__module__, "on_activated", EVENT_TIMEOUT)

def on_deactivated(v):
    for callback in all_callbacks['on_deactivated']:
        run_timed_function(lambda: callback.on_deactivated(v),
            callback.__module__, "on_deactivated", EVENT_TIMEOUT)

def on_project_load(v):
    for callback in all_callbacks['on_project_load']:
        run_timed_function(lambda: callback.on_project_load(v),
            callback.__module__, "on_project_load", EVENT_TIMEOUT)

def on_project_close(v):
    for callback in all_callbacks['on_project_close']:
        run_timed_function(lambda: callback.on_project_close(v),
            callback.__module__, "on_project_close", EVENT_TIMEOUT)

def on_query_context(v, key, operator, operand, match_all):
    for callback in all_callbacks['on_query_context']:
        val = run_timed_function(lambda: callback.on_query_context(v, key, operator, operand, match_all),
            callback.__module__, "on_query_context", FAST_EVENT_TIMEOUT)

        if val:
            return True

    return False

def on_query_completions(v, prefix, locations):
    completions = []
    flags = 0
    for callback in all_callbacks['on_query_completions']:
        res = callback.on_query_completions(v, prefix, locations)

        if isinstance(res, tuple):
            completions += res[0]
            flags |= res[1]
        elif isinstance(res, list):
            completions += res

    return (completions,flags)

class Command(object):
    def name(self):
        clsname = self.__class__.__name__
        name = clsname[0].lower()
        last_upper = False
        for c in clsname[1:]:
            if c.isupper() and not last_upper:
                name += '_'
                name += c.lower()
            else:
                name += c
            last_upper = c.isupper()
        if name.endswith("_command"):
            name = name[0:-8]
        return name

    def is_enabled_(self, args):
        try:
            if args:
                if 'event' in args:
                    del args['event']

                return self.is_enabled(**args)
            else:
                return self.is_enabled()
        except TypeError:
            return self.is_enabled()

    def is_enabled(self):
        return True

    def is_visible_(self, args):
        try:
            if args:
                return self.is_visible(**args)
            else:
                return self.is_visible()
        except TypeError:
            return self.is_visible()

    def is_visible(self):
        return True

    def is_checked_(self, args):
        try:
            if args:
                return self.is_checked(**args)
            else:
                return self.is_checked()
        except TypeError:
            return self.is_checked()

    def is_checked(self):
        return False

    def description_(self, args):
        try:
            if args:
                return self.description(**args)
            else:
                return self.description()
        except TypeError as e:
            return None

    def description(self):
        return None


class ApplicationCommand(Command):
    def run_(self, args):
        if args:
            if 'event' in args:
                del args['event']

            return self.run(**args)
        else:
            return self.run()

    def run(self):
        pass


class WindowCommand(Command):
    def __init__(self, window):
        self.window = window

    def run_(self, args):
        if args:
            if 'event' in args:
                del args['event']

            return self.run(**args)
        else:
            return self.run()

    def run(self):
        pass


class TextCommand(Command):
    def __init__(self, view):
        self.view = view

    def run_(self, args):
        if args:
            if 'event' in args:
                del args['event']

            edit = self.view.begin_edit(self.name(), args)
            try:
                return self.run(edit, **args)
            finally:
                self.view.end_edit(edit)
        else:
            edit = self.view.begin_edit(self.name())
            try:
                return self.run(edit)
            finally:
                self.view.end_edit(edit)

    def run(self, edit):
        pass


class EventListener(object):
    pass
