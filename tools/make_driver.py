#!/usr/bin/env python3
# Author: Alexandr <SCODE>
# Purpose: Formation of archive file for loading in 1C 8.3 configuration.
#
# Usage:
#   write setting access to config.ini, example config.ini.exampl
#   python make_driver.py
# ------------------------------------------------------------------

import argparse
import os
import shutil
import zipfile
import xml.etree.ElementTree as ET
import configparser
from tempfile import TemporaryDirectory


def read_config(config_path):
    config = configparser.ConfigParser()
    config.read(config_path, encoding='utf-8')
    return config

def create_info_xml(config, output_dir):
    root = ET.Element("drivers")
    component = ET.SubElement(root, "component", {
        "progid": config.get("DEFAULT", "progid"),
        "type": config.get("DEFAULT", "type"),
        "name": config.get("DEFAULT", "name"),
        "version": "1.0.0"
    })
    tree = ET.ElementTree(root)
    info_xml_path = os.path.join(output_dir, "info.xml")
    tree.write(info_xml_path, encoding="utf-8", xml_declaration=True)


def create_manifest_xml(config, output_dir):
    root = ET.Element("bundle", {"xmlns": "http://v8.1c.ru/8.2/addin/bundle", "name": "ECRDriverTerminal"})
    path_values = []

    for section in config.sections():
        component_info = {
            "os": None,
            "path": None,
            "type": None,
            "object": None,
            "arch": None,
            "client": None,
            "clientVersion": None,
            "codeType": None
        }
        for key, value in config.items(section):
           if key in component_info:
               component_info[key] = value

        # Collect path values
        if component_info["path"] is not None:
            path_values.append(component_info["path"])

        # Filter out None values
        filedsComponents = { "os" : section }
        filedsComponents.update({k: v for k, v in component_info.items() if v is not None})

        print(f"Write information components: {filedsComponents}")
        ET.SubElement(root, "component", filedsComponents)

    tree = ET.ElementTree(root)
    manifest_xml_path = os.path.join(output_dir, "manifest.xml")
    tree.write(manifest_xml_path, encoding="utf-8", xml_declaration=True)

    return path_values

def copy_files(path_files, src_dir, dest_dir, exclude_dir):
    find_files = []
    for root, dirs, files in os.walk(src_dir):
        print(f"Searching in directory: {root}")
        dirs[:] = [d for d in dirs if not any(e.lower() == d.lower() for e in exclude_dir)]
        for file in files:
            if file in path_files:
                print(f"Copy library: {file}")
                shutil.copy(os.path.join(root, file), dest_dir)
                find_files.append(file)

    missing_files = set(path_files) - set(find_files)
    if missing_files:
        print(f"The following files were not found: {', '.join(missing_files)}")
        return False

    return True

def create_zip(archive_path, src_dir):
    with zipfile.ZipFile(archive_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for file in os.listdir(src_dir):
            zipf.write(os.path.join(src_dir, file), arcname=file)


def main():
    config_path = os.path.join(os.path.dirname(__file__), "config.ini")
    if not os.path.exists(config_path):
        raise FileNotFoundError("Configuration file not found: config.ini")

    parser = argparse.ArgumentParser(description="Creating a driver file for download in the 1C configuration 8.3.")
    parser.add_argument("-build_type", choices=["DEBUG", "RELEASE"], default="DEBUG",
                        help="Тип сборки (DEBUG или RELEASE)")
    parser.add_argument("-build_dir", help="dll build file catalog")
    parser.add_argument("-output_dir", nargs='?', default=os.getcwd(), help="Directory for saving the output archive (default is the current directory)")

    build_dir = None
    output_dir = None
    build_type = None
    try:
        args = parser.parse_args()
        build_dir = args.build_dir
        output_dir = args.output_dir
        build_type = args.build_type
    except SystemExit:
        print("\nError: incorrect parameters.\n Example: make_driver.py DEBUG ../bin/x64/")
        parser.print_help()
        exit(1)

    config = read_config(config_path)
    with TemporaryDirectory() as temp_dir:
        print(f"Create temporary folder catalog {temp_dir}")
        os.makedirs(temp_dir, exist_ok=True)

        create_info_xml(config, temp_dir)
        path_files = create_manifest_xml(config, temp_dir)

        if not copy_files(path_files, build_dir, temp_dir, build_type.lower()):
            return

        output_zip = os.path.join(output_dir, f"ECRDriverPOSTerminal_{build_type}.zip")
        os.makedirs(output_dir, exist_ok=True)
        create_zip(output_zip, temp_dir)
        print(f"Driver AddIn 1C 8 successfully created on archive: {output_zip}")

if __name__ == "__main__":
    main()
