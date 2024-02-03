# generate report for each language progress from Qt *.ts file

import os
import sys
import xml.etree.ElementTree as ET

def parse_ts_file(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    return root

def get_language_progress(root):
    total_message_count = 0
    total_finished_count = 0
    for context in root.iter('context'):
        name = context.find('name').text
        message_count = len(context.findall('message'))
        unfinished_count = len(context.findall('message/translation[@type="unfinished"]'))
        vanished_count = len(context.findall('message/translation[@type="vanished"]'))
        obsolete_count = len(context.findall('message/translation[@type="obsolete"]'))
        finished_count = message_count - unfinished_count - vanished_count - obsolete_count
        total_message_count += message_count - vanished_count - obsolete_count
        total_finished_count += finished_count
    return total_finished_count,total_message_count

def generate_svg(name, finished_count, total_count):
    width = 150
    height = 20
    # generate svg file
    root = ET.Element('svg')
    root.set('xmlns', 'http://www.w3.org/2000/svg')
    root.set('width', str(width))
    root.set('height', str(height))
    root.set('viewBox', '0 0 ' + str(width) + ' ' + str(height))
    root.set('version', '1.1')
    root.set('preserveAspectRatio', 'none')
    root.set('style', 'background-color: #000000')
    # generate finished part
    finished = ET.SubElement(root, 'rect')
    finished.set('x', '0')
    finished.set('y', '0')
    finished.set('width', str(finished_count/total_count*width))
    finished.set('height', str(height))
    finished.set('fill', '#00ff00')
    # generate unfinished part
    unfinished = ET.SubElement(root, 'rect')
    unfinished.set('x', str(finished_count/total_count*width))
    unfinished.set('y', '0')
    unfinished.set('width', str(width-finished_count/total_count*width))
    unfinished.set('height', str(height))
    unfinished.set('fill', '#ff0000')
    # generate finished count
    finished_text = ET.SubElement(root, 'text')
    finished_text.set('x', str(width/2))
    finished_text.set('y', str(height/2))
    finished_text.set('text-anchor', 'middle')
    finished_text.set('dominant-baseline', 'middle')
    finished_text.set('font-size', str(height/1.5))
    finished_text.set('fill', '#000000')
    finished_text.text = str(finished_count)+'/'+str(total_count)+' ('+str(round(finished_count/total_count*100,2))+'%)'
    # write to file
    tree = ET.ElementTree(root)
    tree.write(name + '.svg')

    

if __name__ == '__main__':
    ts_files = []
    for root, dirs, files in os.walk('.'):
        for filename in files:
            if filename.endswith('.ts'):
                ts_files.append(os.path.join(root, filename))
    for ts_file in ts_files:
        total_finished_count,total_message_count = get_language_progress(parse_ts_file(ts_file))
        name = ts_file.split('/')[-1].split('.')[0].replace('quardCRT_','').replace(' ','')
        value = str(total_finished_count)+'/'+str(total_message_count)
        if len(str(total_finished_count)) < 3:
            value = ' '+value
        percentage = str(round(total_finished_count/total_message_count*100,2))+'%'
        print(name+': '+value+' ('+percentage+')')
        generate_svg('../docs/'+ts_file.split('/')[-1].split('.')[0].replace('quardCRT_',''),total_finished_count,total_message_count)
    
