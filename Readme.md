# **Lexan**  
### Word Frequency Analyzer

**Lexan** is a text analysis tool that identifies the **top-k most frequent words** in a given input file. It employs a **master-worker architecture** with three types of processes: **splitters**, **builders**, and a **root** process for efficient computation.

## **Features**  
- **Parallel Processing**: Distributes text analysis across multiple processes to enhance performance.  
- **Word Frequency Analysis**: Computes the frequency of each word in an input file.  
- **Top-k Words Identification**: Outputs the k most frequent words, where k is user-defined.  

## **Architecture**  
The system is composed of the following components:  

1. **Splitters**  
   - Reads segments of the input file.  
   - Processes text and sends words to **builders** using a hash function.  

2. **Builders**  
   - Receives words from splitters.  
   - Updates a hash table to track word frequencies.  
   - Sends results to the **root** process.  

3. **Root (lexan)**  
   - Aggregates data from all builders.  
   - Sorts the word frequencies.  
   - Outputs the **top-k most frequent words**.  


## **Usage**

### Compilation  
Use the provided Makefile to compile the project:  
```bash
make
```

### Execution  
To run the program:  
```bash
./lexan -i TextFile -l numOfSplitter -m numOfBuilders -t TopPopular -e ExclusionList -o OutputFile
```  

### Example  
```bash
./lexan input.txt 4 4 10 exclusion.txt output.txt 
```  
This will output the **top 10 most frequent words** in `example.txt`.  

## **Files**  
- `lexan.c`: Main root process implementation.  
- `splitter.c`: Implementation of the splitter processes.  
- `builder.c`: Implementation of the builder processes.  
- `Makefile`: Compilation script.  

## **License**  
This project is open-source. Feel free to use and modify it.
