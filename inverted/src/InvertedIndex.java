import java.io.IOException;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.HashMap;
import java.util.Set;
import java.util.Map;
import java.lang.*;
import java.util.List;
import java.util.Map.Entry;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.LinkedHashMap;
import java.util.Collections;
import java.util.Comparator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

/**
 * Builds an inverted index: each word followed by files it was found in.
 */
public class InvertedIndex
{

		public static class InvertedIndexMapper extends Mapper<LongWritable, Text, Text, Text>
		{
				private final static Text word = new Text();
				private final static Text location = new Text();

				public void map(LongWritable key, Text val, Context context) throws IOException, InterruptedException
				{
						FileSplit fileSplit = (FileSplit) context.getInputSplit();
						String fileName = fileSplit.getPath().getName();
						location.set(fileName);

						String line = val.toString();
						StringTokenizer itr = new StringTokenizer(line.toLowerCase(), " , .;:'\"&!?-_\n\t12345678910[]{}<>\\`~|=^()@#$%^*/+-");
						while (itr.hasMoreTokens()) {
								word.set(itr.nextToken());
								context.write(word, location);
						}
				}
		}

		public static class InvertedIndexReducer extends Reducer<Text, Text, Text, Text>
		{
				public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException
				{
						boolean first = true;
						int total = 1;
						String filename = "";
						String next = "";
						Iterator<Text> itr = values.iterator();
						//HashMap<Filename, Occurrence count>
						HashMap<String, Integer> map = new HashMap<String, Integer>();
						while (itr.hasNext()) {
								if (first)
								{	
										first = false;
										filename = itr.next().toString();
										if(!itr.hasNext())
												context.write(key, new Text(total + " " + filename));
								}
								else
								{
										if(itr.hasNext())
										{
												next = itr.next().toString();

												if(filename.equals(next))
												{
														//We have nother occurrence of this filename, increment total
														total++;
												}
												else
												{
														//We are at the end of the line so save the filename
														//with corresponding occurrence count in the hashmap
														map.put(filename, total);
														//Move on to the next file name
														filename = next;
														//Reset counter
														total = 1;
												}
										}	
										else
										{
												map.put(filename, total);
												total = 1;	
										}
								}			
						}

						if(!map.isEmpty())
						{
								//Sort the map so that the more occurrences go first in the output
								map = sortByComparator(map ,false);
								//if (key );
								context.write(key, new Text(mapToString(map)));
						}
				}

				/**
				 *  Method mapToString
				 *  Take the values in the hashmap and convert them to a string object
				 *  so that the output matches the requirement.
				 *  @param map - The map containing the list of file names and occurrences
				 *  @return String - The string representing the better indexed version of
				 *  the map reduce for a word occurrences by file.
				 */
				private static String mapToString(HashMap<String, Integer> map)
				{
						StringBuilder toReturn = new StringBuilder();
						for (Map.Entry<String, Integer> entry : map.entrySet()) {
								//Number of occurrences in file
								Integer value = entry.getValue();
								//Filename
								String key = entry.getKey();
								//Output should match: #occurrences filename,
								toReturn.append(value + " " + key + ", ");
						}
						return toReturn.toString();
				}

				/**
				 *  Method sortByComparator
				 *  Takes the hashmap and sorts the map based on the number of occurrences, the value in the hash map.
				 *  @param unsortMap - The unsorted hash map
				 *  @param order - true for ascending, false for descending
				 *  @return sorted - the sorted version of the hash map
				 */
				private static HashMap<String, Integer> sortByComparator(HashMap<String, Integer> unsortMap, final boolean order)
				{

						List<Entry<String, Integer>> list = new LinkedList<Entry<String, Integer>>(unsortMap.entrySet());

						// Sorting the list based on values
						Collections.sort(list, new Comparator<Entry<String, Integer>>()
						{
							public int compare(Entry<String, Integer> o1, Entry<String, Integer> o2)
							{
							  if (order)
							  {
								return o1.getValue().compareTo(o2.getValue());
							  } else {
								return o2.getValue().compareTo(o1.getValue());
							  }
							}
							
						});

						

						// Maintaining insertion order with the help of LinkedList
						HashMap<String, Integer> sortedMap = new LinkedHashMap<String, Integer>();
						for (Entry<String, Integer> entry : list)
						{
								sortedMap.put(entry.getKey(), entry.getValue());
						}

						return sortedMap;
				}

		}

		public static void main(String[] args) throws IOException, ClassNotFoundException, InterruptedException
		{
				Configuration conf = new Configuration();
				if (args.length < 2) {
						System.out.println("Usage: InvertedIndex <input path> <output path>");
						System.exit(1);
				}
				Job job = new Job(conf, "InvertedIndex");
				job.setJarByClass(InvertedIndex.class);
				job.setMapperClass(InvertedIndexMapper.class);
				job.setReducerClass(InvertedIndexReducer.class);
				job.setOutputKeyClass(Text.class);
				job.setOutputValueClass(Text.class);

				FileInputFormat.addInputPath(job, new Path(args[0]));
				FileOutputFormat.setOutputPath(job, new Path(args[1]));
				System.exit(job.waitForCompletion(true) ? 0 : 1);
		}
}