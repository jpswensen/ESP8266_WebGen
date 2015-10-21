import os
import binascii

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
    lambdaText += "\t\tsendBinaryFile(\"text/html\", _index_html, _index_html_len);\n"
    lambdaText += "\t});\n"

    return lambdaText

def generateLambdaFromFilename(filename):
    variableName = generateVariableArrayNameFromFilename(filename)
    lengthName = generateVariableLengthNameFromFilename(filename)
    
    mimeType = getMimeTypeFromFilename(filename)
    
    lambdaText = ""
    lambdaText += "\tserver.on( \"" + filename[1:] + "\", []() {\n"
    lambdaText += "\t\tsendBinaryFile(\"" + mimeType + "\", " + variableName + ", " + lengthName + ");\n"
    lambdaText += "\t});\n"

    return lambdaText

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
        
    
# Get a complete list of files and generate their C-code array equivalents
for (dir, _, files) in os.walk("./"):
     for f in files:
        if f.endswith(".h") or f.endswith(".py") or f.endswith(".DS_Store"):
            continue
         
     	path = os.path.join(dir, f)
     	if os.path.exists(path):
            print generateCCodeFromFile(path)

# Generate the URL handler functions
print generateHandlerSetupPreamble()
print generateRootLambda()
for (dir, _, files) in os.walk("./"):
     for f in files:
        if f.endswith(".h") or f.endswith(".py") or f.endswith(".DS_Store"):
            continue
         
     	path = os.path.join(dir, f)
     	if os.path.exists(path):
            print generateLambdaFromFilename(path)
print generateHandlerSetupPostlude()









    
