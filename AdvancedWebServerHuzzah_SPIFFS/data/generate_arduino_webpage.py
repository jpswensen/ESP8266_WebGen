import os
import binascii
import re

haveLatestFromGithub = 1

def generateVariableArrayNameFromFilename(filename):
    retval = ""
    pathComps = os.path.split(filename[2:])

    temp = "_".join(pathComps)
    retval = re.sub("[\W\d]", "_", temp.strip())
    
    return retval.lower()

def generateVariableLengthNameFromFilename(filename):
    retval = ""
    pathComps = os.path.split(filename[2:])

    temp = "_".join(pathComps)
    retval = re.sub("[\W\d]", "_", temp.strip())
    
    return retval.lower() + "_len"



    
def generateCCodeFromFile(filename):
    # Convert the filename to the C array name
    variableName = generateVariableArrayNameFromFilename(filename)
    lengthName = generateVariableLengthNameFromFilename(filename)

    # Output the C array representing the file    
    retval = "PROGMEM prog_char " + variableName + "[] = {\n"

    counter = 0
    with open(filename, 'rb') as f:
        content = f.read()
        for c in content:
            retval += ' 0x%02x,' % ord(c)
            counter += 1
            if counter % 16 == 0:
                retval += "\n"
                
    retval += "\n};"

    # Output the variable storing the array length
    retval += "\n"
    retval += "unsigned int " + lengthName + " = %d;\n" % counter
    
    return retval

def generateHandlerSetupPreamble():
    retval = "void setupPageHandlers() {\n"
    return retval

def generateHandlerSetupPostlude():
    retval = "}\n\n"
    return retval

def generateRootLambda():
    
    lambdaText = ""
    lambdaText += "\tserver.on( \"/\", []() {\n"

    if haveLatestFromGithub == 1:
        lambdaText += "\t\tserver.send_P(200, htmlMime, _index_html, _index_html_len);"
    else:
        lambdaText += "\t\tsendBinaryFile(\"text/html\", _index_html, _index_html_len);\n"


    lambdaText += "\t});\n"

    return lambdaText

def generateLambdaFromFilename(filename):
    variableName = generateVariableArrayNameFromFilename(filename)
    lengthName = generateVariableLengthNameFromFilename(filename)
    
    mimeType = getMimeTypeFromFilename(filename)
    PROGMEMmimeType = getPROGMEMMimeTypeFromFilename(filename)
    
    lambdaText = ""
    lambdaText += "\tserver.on( \"" + filename[1:] + "\", []() {\n"

    if haveLatestFromGithub == 1:
        lambdaText += "\t\tserver.send_P(200, " + PROGMEMmimeType + ", " +  variableName + ", " + lengthName + ");\n"
    else:
        lambdaText += "\t\tsendBinaryFile(\"" + mimeType + "\", " + variableName + ", " + lengthName + ");\n"
    lambdaText += "\t});\n"

    return lambdaText

def getPROGMEMMimeTypeFromFilename(filename):
    retval = ""
    if filename.endswith("html"):
        retval = "htmlMime"
    elif filename.endswith("css"):
        retval = "cssMime"
    elif filename.endswith("png"):
        retval = "pngMime"
    elif filename.endswith("jpg") or filename.endswith("jpeg"):
        retval = "jpegMime"
    else:
        retval = "plainMime"
        
    return retval

def getMimeTypeFromFilename(filename):
    retval = ""
    if filename.endswith("html"):
        retval = "text/html"
    elif filename.endswith("css"):
        retval = "text/css"
    elif filename.endswith("png"):
        retval = "image/png"
    elif filename.endswith("jpg") or filename.endswith("jpeg"):
        retval = "image/jpeg"
    else:
        retval = "text/plain"
        
    return retval



        

# Print out the extern header
print "extern ESP8266WebServer server;\n\n"

if haveLatestFromGithub == 1:
    print "char plainMime[] PROGMEM = \"text/plain\";"
    print "char htmlMime[] PROGMEM = \"text/html\";"
    print "char cssMime [] PROGMEM = \"text/css\";"
    print "char jsMime[] PROGMEM = \"text/html\";"
    print "char pngMime[] PROGMEM = \"image/png\";"
    print "char jpgMime[] PROGMEM = \"image/jpeg\";\n"



# Get a complete list of files and generate their C-code array equivalents
for (dir, _, files) in os.walk("./"):
     for f in files:
        if f.endswith(".h") or f.endswith(".py") or f.endswith(".DS_Store") or '.' not in f:
            continue
         
     	path = os.path.join(dir, f)
     	if os.path.exists(path):
            print generateCCodeFromFile(path)

# Generate the URL handler functions
print generateHandlerSetupPreamble()
print generateRootLambda()
for (dir, _, files) in os.walk("./"):
     for f in files:
        if f.endswith(".h") or f.endswith(".py") or f.endswith(".DS_Store") or '.' not in f:
            continue
         
     	path = os.path.join(dir, f)
     	if os.path.exists(path):
            print generateLambdaFromFilename(path)
print generateHandlerSetupPostlude()









    
