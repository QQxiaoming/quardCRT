import os
import sys
import csv
import json
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

def check_json_support(support_platform, cc):
    platform_list = support_platform
    os = platform.system()
    if os == 'Windows':
        if cc == 'msvc':
            return 'windows_x86_64_msvc' in platform_list
        return 'windows_x86_64_mingw' in platform_list
    elif os == 'Linux':
        return 'linux_x86_64' in platform_list
    elif os == 'Darwin':
        if platform.machine().startswith('arm'):
            return 'macos_arm64' in platform_list
        return 'macos_x86_64' in platform_list
    else:
        return False

def check_csv_support(support_platform, cc):
    support_platform = support_platform.replace('[', '')
    support_platform = support_platform.replace(']', '')
    platform_list = support_platform.split(';')
    return check_json_support(platform_list)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python get_plugin.py <file> <plugin dir> [cc]')
        exit(1)
    plugin_list_file = sys.argv[1]
    plugin_dir = sys.argv[2]
    if len(sys.argv) == 4:
        cc = sys.argv[3]
    else:
        cc = 'default'

    if plugin_list_file.endswith('.csv'):
        with open(plugin_list_file, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            plugin_list = list(reader)
        for plugin_info in plugin_list:
            plugin_name = plugin_info[0]
            plugin_support_platform = plugin_info[3]
            if not check_csv_support(plugin_support_platform, cc):
                print('Skip plugin: ' + plugin_name)
                continue
            plugin_url = plugin_info[1] + '/' +get_file_name(plugin_info[2], True, cc)
            print('Downloading plugin: ' + plugin_name)
            print('URL: ' + plugin_url)
            plugin_path = os.path.join(plugin_dir, get_file_name(plugin_info[2],False, cc))
            subprocess.run(['curl', '-L', plugin_url, '-o', plugin_path])
            if platform.system() == 'Linux':
                subprocess.run(['chmod', '+x', plugin_path])
            if platform.system() == 'Darwin':
                subprocess.run(['chmod', '+x', plugin_path])

    if plugin_list_file.endswith('.json'):
        with open(plugin_list_file, 'r', encoding='utf-8') as f:
            plugin_list = json.load(f)
        for plugin_info in plugin_list['plugin']:
            plugin_name = plugin_info['name']
            plugin_support_platform = plugin_info['platform']
            if not check_json_support(plugin_support_platform, cc):
                print('Skip plugin: ' + plugin_name)
                continue
            plugin_url = plugin_info['url'] + '/' + get_file_name(plugin_info['filetagname'], True, cc)
            print('Downloading plugin: ' + plugin_name)
            print('URL: ' + plugin_url)
            plugin_path = os.path.join(plugin_dir, get_file_name(plugin_info['filetagname'], False, cc))
            subprocess.run(['curl', '-L', plugin_url, '-o', plugin_path])
            if platform.system() == 'Linux':
                subprocess.run(['chmod', '+x', plugin_path])
            if platform.system() == 'Darwin':
                subprocess.run(['chmod', '+x', plugin_path])

    print('All plugins downloaded.')
