import re


# Note: this should be treated as if it were a static variable inside substitute_system_uuid
# This is an attempt to memoize / cache so that we don't pay the cost of re-compiling the regex each call to
# substitute_system_uuid
matcher = re.compile(r'ZIPPY-SERIAL-NUMBER')

def substitute_system_uuid(thing, system_uuid):
    return matcher.sub(system_uuid, thing)

def substitute_system_uuid_in_system_description(system_description, system_uuid):
    for p in system_description.processes:
        p.executable_path = substitute_system_uuid(p.executable_path, system_uuid)

        for (idx, item) in enumerate(p.arguments):
            p.arguments[idx] = substitute_system_uuid(item, system_uuid)

        keys = [k for k in p.environment]

        # Man I wish the python protobuf bindings were less ridiculous about not behaving like built-in-types where
        # comprehensions, etc. just work.
        for k in keys:
            new_key = substitute_system_uuid(k, system_uuid)
            new_value = substitute_system_uuid(p.environment[k], system_uuid)
            del p.environment[k]
            p.environment[new_key] = new_value

    return system_description

# Make sure that we only export the stuff we intend to
__all__ = ['substitute_system_uuid', 'substitute_system_uuid_in_system_description']