/*  assembler for MarchVM
 *  (code part)
 *  MEDVEDx64, 2013-09-04
 *  MIT license.
 */

package march.utils;

public class MarchAssemblerException extends Exception {
    public static final int ERRTYPE_UNKNOWN_ERROR = 0;
    public static final int ERRTYPE_SYNTAX_ERROR = 1;
    public static final int ERRTYPE_UNKNOWN_INSTRUCTION = 2;
    public static final int ERRTYPE_UNKNOWN_OPERAND = 3;
    
    private int line = 0;
    private int errType = ERRTYPE_UNKNOWN_ERROR;
    public MarchAssemblerException(int numLine) {
        line = numLine;
    }
    
    public MarchAssemblerException(int numLine, int errType) {
        line = numLine;
        this.errType = errType;
    }
    
    @Override public String toString() {
        switch(errType) {
            case ERRTYPE_SYNTAX_ERROR:
                return "Syntax error at line " + line;
            case ERRTYPE_UNKNOWN_INSTRUCTION:
                return "Unknown instruction (line " + line + ")";
            case ERRTYPE_UNKNOWN_OPERAND:
                return "Unknown operand (line " + line + ")";
            default:
                return "Error at line " + line;
        }
    }
}
