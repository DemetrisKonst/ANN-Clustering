import sys
import re

# The following is a simple script to calculate the results of ANN
# Command line arguments are: output file, amount of queries in that file, algorithm used (LSH or HC)

def main():
    if len(sys.argv) != 4:
        print("3 arguments expected (outputFile name, query amount in file, algorithm (LSH or HC))")
        return

    dtRegex = "^distanceTrue"
    ttRegex = "^tTrue"
    if sys.argv[3] == "LSH":
        daRegex = "^distanceLSH"
        taRegex = "^tLSH"
    elif sys.argv[3] == "HC":
        daRegex = "^distanceHypercube"
        taRegex = "^tHypercube"
    else:
        print("Unrecognized algorithm (3rd argument)")

    distanceTrueLines = []
    distanceAlgoLines = []
    timeTrueLines = []
    timeAlgoLines = []
    with open (sys.argv[1], 'rt') as outputFile:
        for line in outputFile:
            if (re.search(daRegex, line)):
                distanceAlgoLines.append(line.rstrip('\n'))
            elif (re.search(dtRegex, line)):
                distanceTrueLines.append(line.rstrip('\n'))
            elif (re.search(taRegex, line)):
                timeAlgoLines.append(line.rstrip('\n'))
            elif (re.search(ttRegex, line)):
                timeTrueLines.append(line.rstrip('\n'))


        daSum = 0
        for line in distanceAlgoLines:
            elements = line.split(' ')
            value = int(elements[1])
            daSum += value

        dtSum = 0
        for line in distanceTrueLines:
            elements = line.split(' ')
            value = int(elements[1])
            dtSum += value

        taSum = 0
        for line in timeAlgoLines:
            elements = line.split(' ')
            value = float(elements[1])
            taSum += value

        ttSum = 0
        for line in timeTrueLines:
            elements = line.split(' ')
            value = float(elements[1])
            ttSum += value

        print("Mean Time Search Algo:", taSum/int(sys.argv[2]))
        print("Mean Time Search BF: ", ttSum/int(sys.argv[2]))
        print("Time Ratio: ", taSum/ttSum)

        print('\n')

        print("Mean Distance Search Algo:", daSum/int(sys.argv[2]))
        print("Mean Distance Search BF:", dtSum/int(sys.argv[2]))
        print("Accuracy: ", (dtSum/daSum)*100, "%")

    outputFile.close()

if __name__ == "__main__":
    main()
