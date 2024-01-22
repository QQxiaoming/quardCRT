# Use python to write and analyze the number of lines, 
# number of comment lines, and number of blank lines of 
# cpp and h files in the source code file directory.

import os
import re
import xml.etree.ElementTree as ET

file_data_list = []

# Get the number of lines, number of comment lines, and number of blank lines of a file
def get_file_data(file_path):
    file_data = {}
    file_data['file_name'] = file_path
    file_data['line_count'] = 0
    file_data['comment_count'] = 0
    file_data['blank_count'] = 0
    file_data['code_count'] = 0
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            file_data['line_count'] += 1
            if re.match(r'^\s*$', line):
                file_data['blank_count'] += 1
            elif re.match(r'^\s*//', line):
                file_data['comment_count'] += 1
            else:
                file_data['code_count'] += 1
    with open(file_path, 'r', encoding='utf-8') as f:
        file_content = f.read()
        comment_list = re.findall(r'/\*.*?\*/', file_content, re.S)
        for comment in comment_list:
            file_data['comment_count'] += comment.count('\n')
            file_data['code_count'] -= comment.count('\n')
    file_data_list.append(file_data)
    return file_data

# Get the number of lines, number of comment lines, and number of blank lines of a directory
def get_dir_data(dir_path):
    dir_data = {}
    dir_data['dir_name'] = dir_path
    dir_data['line_count'] = 0
    dir_data['comment_count'] = 0
    dir_data['blank_count'] = 0
    dir_data['code_count'] = 0
    if dir_path == '../src/plugin':
        return dir_data
    lsdir = os.listdir(dir_path)
    for fe in lsdir:
        file_path = os.path.join(dir_path, fe)
        if not re.match(r'.*\.(cpp|h)$', file_path):
            continue
        file_data = get_file_data(file_path)
        dir_data['line_count'] += file_data['line_count']
        dir_data['comment_count'] += file_data['comment_count']
        dir_data['blank_count'] += file_data['blank_count']
        dir_data['code_count'] += file_data['code_count']
    for dr in lsdir:
        dirp = os.path.join(dir_path, dr)
        if not os.path.isdir(dirp):
            continue
        subdir_data = get_dir_data(dirp)
        dir_data['line_count'] += subdir_data['line_count']
        dir_data['comment_count'] += subdir_data['comment_count']
        dir_data['blank_count'] += subdir_data['blank_count']
        dir_data['code_count'] += subdir_data['code_count']
    return dir_data

def generate_html(name, file_data_list):
    # Generate table
    # | file_name | line_count | comment_count | blank_count | code_count |
    # | file_data['file_name'] | file_data['line_count'] | file_data['comment_count'] | file_data['blank_count'] | file_data['code_count'] |
    # Generate html file
    root = ET.Element('html')
    # Generate head
    head = ET.SubElement(root, 'head')
    # Generate body
    body = ET.SubElement(root, 'body')
    # Generate table
    table = ET.SubElement(body, 'table')
    # Generate table head
    table_head = ET.SubElement(table, 'thead')
    # Generate table body
    table_body = ET.SubElement(table, 'tbody')
    # Generate table head row
    table_head_row = ET.SubElement(table_head, 'tr')
    # Generate table head row data
    table_head_row_data = ET.SubElement(table_head_row, 'td')
    table_head_row_data.text = 'file_name'
    table_head_row_data = ET.SubElement(table_head_row, 'td')
    table_head_row_data.text = 'line_count'
    table_head_row_data = ET.SubElement(table_head_row, 'td')
    table_head_row_data.text = 'comment_count'
    table_head_row_data = ET.SubElement(table_head_row, 'td')
    table_head_row_data.text = 'blank_count'
    table_head_row_data = ET.SubElement(table_head_row, 'td')
    table_head_row_data.text = 'code_count'
    for file_data in file_data_list:
        # Generate table body row
        table_body_row = ET.SubElement(table_body, 'tr')
        # Generate table body row data
        table_body_row_data = ET.SubElement(table_body_row, 'td')
        table_body_row_data.text = file_data['file_name']
        table_body_row_data = ET.SubElement(table_body_row, 'td')
        table_body_row_data.text = str(file_data['line_count'])
        table_body_row_data = ET.SubElement(table_body_row, 'td')
        table_body_row_data.text = str(file_data['comment_count'])
        table_body_row_data = ET.SubElement(table_body_row, 'td')
        table_body_row_data.text = str(file_data['blank_count'])
        table_body_row_data = ET.SubElement(table_body_row, 'td')
        table_body_row_data.text = str(file_data['code_count'])
    # Generate html file
    tree = ET.ElementTree(root)
    tree.write('code_analyze.html', encoding='utf-8', xml_declaration=True)

if __name__ == '__main__':
    dir_path = '../src'
    dir_data = get_dir_data(dir_path)
    print(dir_data)
    dir_path = '../lib'
    dir_data = get_dir_data(dir_path)
    print(dir_data)
    # sort by code_count
    file_data_list.sort(key=lambda x:x['code_count'], reverse=True)
    total = {}
    total['file_name'] = "total"
    total['line_count'] = 0
    total['comment_count'] = 0
    total['blank_count'] = 0
    total['code_count'] = 0
    for file_data in file_data_list:
        total['line_count'] += file_data['line_count']
        total['comment_count'] += file_data['comment_count']
        total['blank_count'] += file_data['blank_count']
        total['code_count'] += file_data['code_count']
    file_data_list.insert(0, total)
    generate_html(dir_data['dir_name'], file_data_list)
