from sys import argv


for arg in argv[1:]:
    with open(arg+".h", "w") as fout:
        filename = arg.replace('.', '_')
        line_names = []       
        for i, line in enumerate(open(arg)):
            line_name = "{}_{}".format(filename, i)
            fout.write('const char {} [] PROGMEM = "{}";\n'.format(line_name, line.strip().replace('"', '\\"')))
            line_names.append(line_name)
        
        fout.write('const int {}_size={};\n'.format(filename, len(line_names)))    
        fout.write(' const char * const {}[] PROGMEM = {{'.format(filename))
        fout.write(",\n".join(line_names))
        fout.write('};')

        
            
