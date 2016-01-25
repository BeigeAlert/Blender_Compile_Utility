#include <string>
#include <vector>

const std::wstring XmlFileName = L"builder_setup.xml";
const std::vector<const std::wstring> XmlEntries = {
                                                    L"<!-- begin blender stuff -->\n",
                                                    L"<!-- exclude blender backup files -->\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend1</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend2</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend3</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend4</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend5</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend6</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend7</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend8</match></rule>\n",
                                                    L"<rule><match>${src_dir}\\{{*\\}?*}.blend9</match></rule>\n",
                                                    L"\n",
                                                    L"<rule> <!-- blender files in modelsrc -->\n",
                                                    L"\t<match>${src_dir}\\modelsrc\\{{*/}?*}.blend</match>\n",
                                                    L"\t<output_file>${dst_dir}\\models\\%1.blend</output_file>\n",
                                                    L"\t<command>\"${builder_dir}\\BlenderCompiler\" \"${src_dir}\\modelsrc\\%1.blend\"</command>\n",
                                                    L"</rule>\n",
                                                    L"\n",
                                                    L"<rule> <!-- blender files -->\n",
                                                    L"\t<match>${src_dir}\\{{*/}?*}.blend</match>\n",
                                                    L"\t<output_file>${dst_dir}\\%1.blend</output_file>\n",
                                                    L"\t<command>\"${builder_dir}\\BlenderCompiler\" \"${src_dir}\\%1.blend\"</command>\n",
                                                    L"</rule>\n",
                                                    L"<!-- end blender stuff -->\n",
                                                    };