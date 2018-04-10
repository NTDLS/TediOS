@Echo Off

attrib -s -h -r

Del *.Ncb
Del *.Opt
Del *.Plg
Del *.Aps
Del *.Scc
Del *.suo
Del *.xml
Del *.old
Del *.user

Del Release\*.* /Q
Del Debug\*.* /Q
Del Setup\Output\*.* /Q

RD Release
RD Debug
RD Setup\Output

Del Resources\*.aps

Cls