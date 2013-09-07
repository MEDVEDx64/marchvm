/*  assembler for MarchVM
 *  (code part)
 *  MEDVEDx64, 2013-09-04
 *  MIT license.
 */
package march.utils;

import java.util.ArrayList;

public class MarchAssemblerLabelRegistry {
    private ArrayList<String> labels;
    private ArrayList<Character> indexes;
    
    public MarchAssemblerLabelRegistry() {
        labels = new ArrayList<String>();
        indexes = new ArrayList<Character>();
    }
    
    public void addLabel(String label, char index) {
        labels.add(label);
        indexes.add(index);
    }
    
    public boolean indexExist(char index) {
        return indexes.contains(index);
    }
    
    public char findLabel(String label) {
        if(labels.isEmpty() || indexes.isEmpty())
            return 0xffff;
        
        for(int i = 0; i < labels.size(); i++)
            if(labels.get(i).equalsIgnoreCase(label))
                return indexes.get(i);
        return 0xffff;
    }
    
    @Override public String toString() {
        String res = null;
        if(labels.isEmpty())
            return "Empty";
        else {
            res = labels.size() + " total\n";
            for(int i = 0; i < labels.size(); i++)
                res = res + labels.get(i) + ":"
                        + (int)indexes.get(i) + "\n";
            return res;
        }
    }
}
