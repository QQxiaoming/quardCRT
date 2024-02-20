import os
import sys
import csv
import platform
import subprocess

def get_file_name(fname, withmachine, cc):
    os = platform.system()
    if os == 'Windows':
        if cc == 'msvc':
            if(withmachine):
                return fname+'.msvc.dll'
        return fname+'.dll'
    elif os == 'Linux':
        return 'lib'+fname+'.so'
    elif os == 'Darwin':
        if platform.machine().startswith('arm'):
            if(withmachine):
                return 'lib'+fname+'.arm64.dylib'
        return 'lib'+fname+'.dylib'
    else:
        return 'unknown'

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python get_plugin.py <csv file> <plugin dir> [cc]')
        exit(1)
    plugin_list_file = sys.argv[1]
    plugin_dir = sys.argv[2]
    if len(sys.argv) == 4:
        cc = sys.argv[3]
    else:
        cc = 'default'
    with open(plugin_list_file, 'r', encoding='utf-8') as f:
        reader = csv.reader(f)
        plugin_list = list(reader)
    for plugin_info in plugin_list:
        plugin_name = plugin_info[0]
        plugin_url = plugin_info[1] + '/' +get_file_name(plugin_info[2], True, cc)
        print('Downloading plugin: ' + plugin_name)
        print('URL: ' + plugin_url)
        plugin_path = os.path.join(plugin_dir, get_file_name(plugin_info[2],False, cc))
        subprocess.run(['curl', '-L', plugin_url, '-o', plugin_path])
        if platform.system() == 'Linux':
            subprocess.run(['chmod', '+x', plugin_path])
        if platform.system() == 'Darwin':
            subprocess.run(['chmod', '+x', plugin_path])
    print('All plugins downloaded.')

    