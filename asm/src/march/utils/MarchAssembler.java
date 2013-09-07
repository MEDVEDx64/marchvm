/*  assembler for MarchVM
 *  MEDVEDx64, 2013-09-06
 *  MIT license.
 */

package march.utils;
import java.io.*;
import java.util.ArrayList;

public class MarchAssembler {
    static MarchAssemblerLabelRegistry registry =
            new MarchAssemblerLabelRegistry();
    static ArrayList<String> cache = new ArrayList<String>();
    static char[] registers = new char[4];
    
    static int line = 0;
    static boolean thereIsErrors = false;
    
    static void reset() {
        for(int i = 0; i < 4; i++)
            registers[i] = 0;
    }
    
    static boolean eliminate(String str)
            throws MarchAssemblerException {
        reset();
        if(str == null) return false;
        if(str.length() == 0) return false;
        if(!Character.isLetter(str.charAt(0))) {
            if(str.charAt(0) == '.')
                return true;
            thereIsErrors = true;
            throw new MarchAssemblerException(line+1,
                    MarchAssemblerException.ERRTYPE_SYNTAX_ERROR);
        }
        
        String[] ops = str.split("[ \t]+");
        for(int i = 0; i < ((ops.length <= 4)?
                ops.length: 4); i++) {
            if(ops[i].length() == 0) continue;
            if(i == 0) {
                if(Character.isDigit(ops[i].charAt(0))) {
                    thereIsErrors = true;
                    throw new MarchAssemblerException(line+1,
                            MarchAssemblerException.ERRTYPE_SYNTAX_ERROR);
                }
                else {
                    if(ops[i].equalsIgnoreCase("np"))
                        registers[i] = 0;
                    else if(ops[i].equalsIgnoreCase("ex"))
                        registers[i] = 1;

                    else if(ops[i].equalsIgnoreCase("cp"))
                        registers[i] = 128;
                    else if(ops[i].equalsIgnoreCase("gt"))
                        registers[i] = 256;
                    else if(ops[i].equalsIgnoreCase("pt"))
                        registers[i] = 288;

                    else if(ops[i].equalsIgnoreCase("ad"))
                        registers[i] = 1024;
                    else if(ops[i].equalsIgnoreCase("sb"))
                        registers[i] = 1536;
                    else if(ops[i].equalsIgnoreCase("mp"))
                        registers[i] = 2048;
                    else if(ops[i].equalsIgnoreCase("dv"))
                        registers[i] = 2560;

                    else if(ops[i].equalsIgnoreCase("an"))
                        registers[i] = 5192;
                    else if(ops[i].equalsIgnoreCase("or"))
                        registers[i] = 5320;
                    else if(ops[i].equalsIgnoreCase("tr"))
                        registers[i] = 5448;
                    else if(ops[i].equalsIgnoreCase("tl"))
                        registers[i] = 5576;

                    else if(ops[i].equalsIgnoreCase("ju"))
                        registers[i] = 10240;
                    else if(ops[i].equalsIgnoreCase("ig"))
                        registers[i] = 10272;

                    else if(ops[i].equalsIgnoreCase("gg"))
                        registers[i] = 65471;
                    
                    else {
                        thereIsErrors = true;
                        throw new MarchAssemblerException(line+1,
                            MarchAssemblerException.ERRTYPE_UNKNOWN_INSTRUCTION);
                    }
                    
                }
            } else {
                char lab_idx = registry.findLabel(ops[i]);
                if(lab_idx != 0xffff) {
                    registers[i] = lab_idx;
                    continue;
                }

                if(ops[i].equalsIgnoreCase("cc"))
                    registers[i] = 65534;
                else if(ops[i].equalsIgnoreCase("ll"))
                    registers[i] = 65532;
                else if(ops[i].equalsIgnoreCase("cr"))
                    registers[i] = 65530;

                else if(ops[i].equalsIgnoreCase("a1"))
                    registers[i] = 65529;
                else if(ops[i].equalsIgnoreCase("a2"))
                    registers[i] = 65528;
                else if(ops[i].equalsIgnoreCase("a3"))
                    registers[i] = 65327;

                else if(ops[i].equalsIgnoreCase("d1"))
                    registers[i] = 65514;
                else if(ops[i].equalsIgnoreCase("d1a"))
                    registers[i] = 65513;
                else if(ops[i].equalsIgnoreCase("d1b"))
                    registers[i] = 65512;

                else if(ops[i].equalsIgnoreCase("d2"))
                    registers[i] = 65510;
                else if(ops[i].equalsIgnoreCase("d2a"))
                    registers[i] = 65509;
                else if(ops[i].equalsIgnoreCase("d2b"))
                    registers[i] = 65508;

                else if(ops[i].equalsIgnoreCase("d3"))
                    registers[i] = 65506;
                else if(ops[i].equalsIgnoreCase("d3a"))
                    registers[i] = 65505;
                else if(ops[i].equalsIgnoreCase("d3b"))
                    registers[i] = 65504;

                else if(ops[i].equalsIgnoreCase("d4"))
                    registers[i] = 65502;
                else if(ops[i].equalsIgnoreCase("d4a"))
                    registers[i] = 65501;
                else if(ops[i].equalsIgnoreCase("d4b"))
                    registers[i] = 65500;

                else if(Character.isLetter(ops[i].charAt(0))) {
                    thereIsErrors = true;
                    throw new MarchAssemblerException(line+1,
                            MarchAssemblerException.ERRTYPE_UNKNOWN_OPERAND);
                } else {
                    int candidate = Integer.decode(ops[i]);
                    if((char)candidate < 65472)
                        registers[i] = (char)candidate;
                    else {
                        thereIsErrors = true;
                        throw new MarchAssemblerException(line+1,
                            MarchAssemblerException.ERRTYPE_UNKNOWN_OPERAND);
                    }
                }
            }
        
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        
        BufferedReader in = new BufferedReader(new
                InputStreamReader(System.in));
        
        try {
            while(true) {
                String str = in.readLine();
                if(str == null)
                    break;
                else
                    cache.add(str);
            }
        } catch(IOException e) {
            System.out.println("Read error or reached end of file.");
        }
        
        ArrayList<String> newcache = new ArrayList<String>();
        
        for(int z = 0; z < cache.size(); z++) {
            String tmp = cache.get(z).toString();
            if(tmp == null) continue;
            if(tmp.length() == 0) continue;
            if(tmp.indexOf('#') == -1)
                cache.set(z, tmp);
            else
                cache.set(z, tmp.substring(0, tmp.indexOf('#')));
            
            if(cache.get(z) == null) continue;
            if(cache.get(z).length() == 0) continue;
            if(cache.get(z).charAt(0) == '.') {
                int a = 1;
                while(true) {
                    if(a >= cache.get(z).length()) break;
                    if(!Character.isLetter(cache.get(z).charAt(a))
                            && cache.get(z).charAt(a) != '_') break;
                    a++;
                }
                registry.addLabel(cache.get(z).substring(1, a), '\0');
            }
            
        }
        
        ArrayList<String> errors;
        for(line = 0; line < cache.size(); line++) {
            try {
                if(eliminate(cache.get(line)))
                    newcache.add(cache.get(line));
            } catch(MarchAssemblerException e) {
                System.err.println(e);
            }
        }
        
        if(thereIsErrors)
            System.exit(-1);
        
        MarchAssemblerLabelRegistry newreg =
                new MarchAssemblerLabelRegistry();
        for(int z = 0; z < newcache.size(); z++)
            if(newcache.get(z).charAt(0) == '.')
                newreg.addLabel(newcache.get(z).substring(1).trim(), (char)z);
        registry = newreg;
        
        DataOutputStream out = new DataOutputStream(System.out);
        try {
            for(int z = 0; z < newcache.size(); z++) {
                if(eliminate(newcache.get(z))) {
                    for(int i = 0; i < 4; i++)
                        out.writeChar((((registers[i])&0xff)<<8)|
                                (((registers[i])&0xff00)>>8));
                                // big endian -> little endian
                    out.flush();
                }
            }
        } catch(IOException e) {
            System.err.println("Write error.");
            System.exit(-1);
        } catch(MarchAssemblerException e) {
            System.err.println("Fatal generate-stage exception: " + e);
            System.exit(-1);
        }
        
    }
}