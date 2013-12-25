﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace ProvinceMapper
{
    class MappingReader
    {
        public List<Mapping> mappings = new List<Mapping>();

        public string srcTag;
        public string destTag;

        private string path;

        public MappingReader(string _path, string _srcTag, string _destTag)
        {
            srcTag = _srcTag;
            destTag = _destTag;
            path = _path;
        }

        public MappingReader(string _path, string _srcTag, string _destTag, List<Province> srcProvs, List<Province> destProvs, StatusUpdate su)
        {
            srcTag = _srcTag;
            destTag = _destTag;
            path = _path;

            StreamReader sr = new StreamReader(path, Encoding.GetEncoding(1252));
            while (!sr.EndOfStream)
            {
                string line = sr.ReadLine().Trim();
                if (line.Length > 0)
                {
                    if (line.StartsWith("link"))
                    {
                        try
                        {
                            mappings.Add(new ProvinceMapping(line, srcTag, destTag, srcProvs, destProvs));
                        }
                        catch (Exception e)
                        {
                            System.Windows.Forms.MessageBox.Show(e.Message, "Error in mapping file");
                        }
                    }
                    else if (line.StartsWith("#"))
                    {
                        try
                        {
                            mappings.Add(new CommentMapping(line));
                        }
                        catch (Exception e)
                        {
                            System.Windows.Forms.MessageBox.Show(e.Message, "Error in mapping file");
                        }
                    }
                    else if (line.StartsWith("mappings") || line.StartsWith("{") || line.StartsWith("}"))
                    {
                        // ignore these lines
                    }
                    else
                    {
                        System.Windows.Forms.MessageBox.Show(String.Format("Error parsing province mapping file: line beginning '{0}' appears to be invalid.", line.Split(' ')[0]));
                    }
                }
                su(100.0 * sr.BaseStream.Position / sr.BaseStream.Length);
            }
            sr.Close();
        }

        public void Write()
        {
            Write(path);
        }

        public void Write(string path)
        {
            StreamWriter sw = new StreamWriter(path, false, Encoding.GetEncoding(1252));
            sw.WriteLine("mappings = {");
            foreach (Mapping m in mappings)
            {
                sw.WriteLine(m.ToOutputString(srcTag, destTag));
            }
            sw.WriteLine("}");
            sw.Close();
        }
    }
}
