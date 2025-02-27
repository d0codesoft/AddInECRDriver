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

    for section in config.sections():
       for key, value in config.items(section):
          arch = key
          path = value
          ET.SubElement(root, "component", {"os": section, "path": path, "type": "native", "arch": arch})

    tree = ET.ElementTree(root)
    manifest_xml_path = os.path.join(output_dir, "manifest.xml")
    tree.write(manifest_xml_path, encoding="utf-8", xml_declaration=True)


def copy_files(src_dir, dest_dir, extensions):
    for file in os.listdir(src_dir):
        if file.endswith(extensions):
            shutil.copy(os.path.join(src_dir, file), dest_dir)


def create_zip(archive_path, src_dir):
    with zipfile.ZipFile(archive_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for file in os.listdir(src_dir):
            zipf.write(os.path.join(src_dir, file), arcname=file)


def main(build_dir="bin", output_dir="output", build_type="Release"):
    config_path = os.path.join(os.path.dirname(__file__), "config.ini")
    if not os.path.exists(config_path):
        raise FileNotFoundError("Файл конфигурации не найден: config.ini")

    parser = argparse.ArgumentParser(description="Формирование файла драйвера для загрузки в конфигурации 1С 8.3.")
    parser.add_argument("--build_type", choices=["DEBUG", "RELEASE"], default="DEBUG",
                        help="Тип сборки (DEBUG или RELEASE)")
    parser.add_argument("build_dir", help="Каталог сохранения архива файла драйвера")
    args = parser.parse_args()

    config = read_config(config_path)
    with TemporaryDirectory() as temp_dir:
        create_info_xml(config, temp_dir)
        create_manifest_xml(config, temp_dir)

        extensions = (".dll", ".so", ".dylib")
        copy_files(build_dir, temp_dir, extensions)

        output_zip = os.path.join(output_dir, f"release_{build_type}.zip")
        os.makedirs(output_dir, exist_ok=True)
        create_zip(output_zip, temp_dir)
        print(f"Package created: {output_zip}")


if __name__ == "__main__":
    main()
