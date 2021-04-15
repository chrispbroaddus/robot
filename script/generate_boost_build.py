#!/usr/bin/env python3

import collections
import os
import fileinput
from pathlib import Path
import re
import subprocess

class BoostLibrary(object):
    def __init__(self, name, library=None):
        self.dependencies = {}
        self.name = name
        self.library = library
        self.headers = {}

    def add_dependency(self, dep):
        if dep != self.name and dep not in self.dependencies:
            self.dependencies[dep] = dep

    def add_header(self, header):
        if header not in self.headers:
            self.headers[header] = header

    def add_dependencies(self, deps):
        for x in deps:
            self.add_dependency(x)


    def as_build_script(self):
        binary_template = """
cc_library(
  name="{}",
  hdrs = {},
  srcs = ["{}"],
  deps = [{}],
  visibility = ["//visibility:public"],
)
"""
        header_template = """
cc_inc_library(
  name="{}",
  hdrs = {},
  deps = [{}],
  visibility = ["//visibility:public"],
)
"""

        fileSuffixes = ['**/*.hpp"', '**/*.h"']
        globPrefix = '"include/boost/' + self.name + '/'
        
        topLevelHeader = Path('/usr/include/boost/') / (self.name + '.hpp')

        headerFragments = []

        if topLevelHeader.exists():
            headerFragments.append('["include/boost/' + self.name + '.hpp"]')

        if Path('/usr/include/boost/' + self.name).exists():
            headerFragments.append('glob([' + ", ".join([globPrefix + x for x in fileSuffixes]) + '])')
        
        headers = ' + '.join(headerFragments)
        deps = ",\n    ".join(['"@boost//:' + x + '"' for x in sorted(self.dependencies.keys())])
        name = self.name


        if len(headerFragments) > 0:
            if self.library is None:
                return header_template.format(name, headers, deps)
            else:
                return binary_template.format(name, headers, str(self.library)[5:], deps)
        else:
            return "# " + self.name
            
def scanBinaryLibraries():
    extension = '.so'
    pattern = 'libboost_*'

    baseDirs = collections.deque()
    baseDirs.append(Path('/usr/lib'))

    boostLibs = {}
    matcher = re.compile(r'libboost_([a-z0-9]+)(-py[0-9]*)?\.so.*')
    matcher2 = re.compile(r'\s+libboost_([a-z0-9]+)(-py[0-9]*)?\.so.* =>')

    while len(baseDirs) > 0:
        toExplore = baseDirs.popleft()

        for entry in os.scandir(os.fsencode(str(toExplore.expanduser().resolve()))):
            item = toExplore / os.fsdecode(entry.name)

            if entry.is_file() and not entry.is_symlink():
                m = matcher.match(item.name)

                if m:
                    libName = m.group(1)

                    if "wserialization" != libName:
                        library = BoostLibrary(m.group(1), library=item)

                        ldd = subprocess.run(['ldd', str(item)], universal_newlines=True, stdout=subprocess.PIPE)
                        lines = ldd.stdout.split('\n')

                        for l in lines:
                            m2 = matcher2.match(l)

                            if m2:
                                depName = m2.group(1)
                                library.add_dependency(depName)

                        boostLibs[libName] = library


            elif entry.is_dir() and not entry.is_symlink():
                baseDirs.append(item)

    return boostLibs


def extractIncludeDependencies(fileName):
    matcher = re.compile(r'^\s*\#include\s+[<"]([^>"]+)[>"]')
    results = {}
    

    print('Processing [{}]'.format(str(fileName)))

    try:
        with fileinput.input(files=(str(fileName))) as file:
            for line in file:
                if line.find('#include') >= 0:
                    #print('[{}] has include [{}]'.format(str(fileName), line.strip()))
                    m = matcher.match(line)

                    if m:
                        #print('[{}] <== [{} @ {}] ***'.format(str(fileName), m.group(1), file.filelineno()))
                        item = Path(m.group(1))
                        results[item] = item
    except KeyboardInterrupt:
        raise
    except:
        pass

    return sorted(results.keys(), key=lambda x:str(x))

def updateHeaderLibraries(boostLibs):
    baseDirs = collections.deque()
    baseDirs.append((None, Path('/usr/include/boost')))

    while len(baseDirs) > 0:
        (libName, toExplore) = baseDirs.popleft()

        for entry in os.scandir(os.fsencode(str(toExplore.expanduser().resolve()))):
            item = toExplore / os.fsdecode(entry.name)

            if entry.is_file():
                targetLibrary = item.stem if libName is None else libName
                includes = extractIncludeDependencies(item)
                dependencies = {}
                
                for x in includes:
                    if str(x).find('boost') >= 0:
                        n = len(x.parents)
                        dep = ''
                        if 2 == n:
                            dep = x.stem
                        else:
                            dep = x.parents[n - 3].stem

                        if dep != 'boost' and dep != 'detail':
                            dependencies[dep] = dep
                        else:
                            print('*** PROBLEM: [{}] <-- [{}] PARENTS = n={}  [{}]'.format(targetLibrary, x, len(x.parents), "// ".join([str(z) for z in x.parents])))
                            

                if targetLibrary not in boostLibs:
                    boostLibs[targetLibrary] = BoostLibrary(targetLibrary)

                #print('Updating [{}] :: [{}] / [{}] @ [{}] deps = [{} @@@ {}]'.format(targetLibrary, libName, item.stem, str(item), ', '.join(sorted([str(x) for x in includes])), ', '.join(sorted([str(x) for x in dependencies.keys()]))))
                
                boostLibs[targetLibrary].add_dependencies(dependencies.keys())
                boostLibs[targetLibrary].add_header(item)

            elif entry.is_dir() and not entry.is_symlink():
                print('Scheduling directory [{}]'.format(str(item.resolve())))
                if libName is None and item.stem != 'detail':
                    # Top level library directory
                    baseDirs.append((os.fsdecode(entry.name), item))
                else:
                    # Somewhere within a library's include hierarchy
                    baseDirs.append((libName, item))

    return boostLibs

def main():
    boostLibs = scanBinaryLibraries()
    boostLibs = updateHeaderLibraries(boostLibs)

    sortedLibs = sorted(boostLibs.values(), key=lambda x: x.name)
    
    with open('boost.BUILD', 'wt') as out:
        print("""
# This file was automatically generated by generate_boost_build.py
#
# Please do not edit it by hand. 



        """, file=out);
        
        for x in sortedLibs:
            print(x.as_build_script(), file=out)

if "__main__" == __name__:
    main()
