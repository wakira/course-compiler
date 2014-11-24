#include "present.h"
#include <iostream>
#include <sstream>

using namespace std;

string getOp(BinaryOperation::Ops op)
{
    switch (op) {
        case BinaryOperation::P:
            return "+";
            break;
        case BinaryOperation::S:
            return "-";
            break;
        case BinaryOperation::M:
            return "*";
            break;
        case BinaryOperation::D:
            return "/";
            break;
        case BinaryOperation::MOD:
            return "%";
            break;
        case BinaryOperation::E:
            return "==";
            break;
        case BinaryOperation::NE:
            return "!=";
            break;
        case BinaryOperation::L:
            return "<";
            break;
        case BinaryOperation::LE:
            return "<=";
            break;
        case BinaryOperation::G:
            return ">";
            break;
        case BinaryOperation::GE:
            return ">=";
            break;
        case BinaryOperation::A:
            return "&&";
            break;
        case BinaryOperation::O:
            return "||";
            break;
        default:
            return "NOP";
            break;
            
    }

}

string dfs(ASTNode *root, int depth)
{
    stringstream ret;
    if (!root)
        return sret.str();
    string indent("");
    for (int i = 0; i < depth; ++i) {
        indent += TAB;
    }
    if (IS_PANY(root, Program)) {
        P_TRANS(node, root, Program);
        ret << indent << "Program Node <" << node->name->name << ">" << endl;
        ret << dfs(node->definitions, depth + 1); 
        ret << dfs(node->variableDeclarations, depth + 1);
        ret << dfs(node->programBlock, depth + 1);
    } else if (IS_PANY(root, VariableDeclaration)) {
        P_TRANS(node, root, VariableDeclaration);
        ret << indent
            << "Variable# type: <" << node->type->name << "> name: <" << node->name->name + ">" << endl;
    } else if (IS_PANY(root, ArrayDefinition)) {
        P_TRANS(node, root, VariableDeclaration);
        ret << indent
            << "Array# type: <" << node->type->name
            << "> name: <"
            << node->name->name
            << "> size: " << node->size << endl;
    } else if (IS_PANY(root, ClassDefinition)) {
        P_TRANS(node, root, ClassDefinition);
        ret << indent << "Class# name: <" << node->className->name << flush;
        if (node->extFrom != nullptr) {
            ret << "> parent: <" << node->extFrom->name << flush;
        }
        ret << ">: " << endl;
        ret << dfs(node->variables, depth + 1);
        ret << dfs(node->functions, depth + 1);
    } else if (IS_PANY(root, FunctionDefinition)) {
        P_TRANS(node, root, FunctionDefinition);
        ret << indent << "Function# name: <" << node->name->name << flush;
        if (node->retType != nullptr) {
            ret << "> return <" << node->retType->name << flush;
        }
        ret << ">: " << endl;
        ret << dfs(node->arguments, depth + 1);
        ret << dfs(node->variables, depth + 1);
        ret << dfs(node->functionBlocks, depth + 1);
    } else if (IS_PANY(root, Primary)) {
        P_TRANS(node, root, Primary);
        ret << dfs(node->expr, depth) << flush;
    } else if (IS_PANY(root, IdentPr)) {
        P_TRANS(node, root, IdentPr);
        ret << indent << node->name->name << endl;
    } else if (IS_PANY(root, ArrayPr)) {
        P_TRANS(node, root, ArrayPr);
        ret << dfs(node->name, depth);
            << dfs(node->index, depth + 1);
    } else if (IS_PANY(root, NumericLiteral)) {
        P_TRANS(node, root, NumericLiteral);
        ret << node->val << endl;
    } else if (IS_PANY(root, BinaryOperation)) {
        P_TRANS(node, root, BinaryOperation);
        ret << indent << getOp(node->op) << endl;
        ret << indent << dfs(node->a, depth + 1);
        ret << indent << dfs(node->b, depth + 1);
    } else if (IS_PANY(root, UnaryOperation)) {
        P_TRANS(node, root, UnaryOperation);
        ret << indent << "(Unary)" << getOp(node->op) << endl;
        ret << dfs(node->p);
    } else if (IS_PANY(root, FunCall)) {
        P_TRANS(node, root, FunCall);
        ret << indent << "Call# name: <" << dfs(node->name, depth + 1)
            << indent << ">: " << endl;
        ret << dfs(node->args, depth + 1) << endl;
    } else if (IS_PANY(root, DotOperation)) {
        P_TRANS(node, root, DotOperation);
        ret << indent << "." << endl;
        ret << dfs(node->pr, depth + 1);
        ret << indent << TAB << node->field->name << endl;
    } else if (IS_PANY(root, ElementList)) {
        P_TRANS(node, root, ElementList);
        ret << indent << "List: " << endl;
        for (int i = 0; i < node->elements.size(); ++i) {
            ret << dfs(node->elements[i], depth + 1);
        }
    } else if (IS_PANY(root, RetStatement)) {
        P_TRANS(node, root, RetStatement);
        ret << indent << "Return# " << endl;
        ret << dfs(node->expr, depth + 1);
    } else if (IS_PANY(root, AssignmentStatement)) {
        P_TRANS(node, root, AssignmentStatement);
        ret << indent << "Assign# " << endl;
        ret << dfs(node->lhs, depth + 1);
        ret << dfs(node->rhs, depth + 1);
    } else if (IS_PANY(root, IOStatement)) {
        P_TRANS(node, root, IOStatement);
        ret << indent;
        if (node->op == IOStatement::IN) {
            ret << "input#" << endl;
        } else {
            ret << "output#" << endl;
        }
        ret << dfs(node->content, depth + 1);
        ret << dfs(node->var, depth + 1);
    } else if (IS_PANY(root, IfStatement)) {
        P_TRANS(node, root, IfStatement);
        ret << indent << "if#" << endl;
        for (int i = 0; i < conds.size(); ++i) {
            if (node->conds[i] == nullptr) {
                ret << indent << TAB << "true" << endl;
            } else {
                ret << dfs(node->conds[i], depth + 1);
            }
            ret << dfs(node->stats[i], depth + 1);
        }
    } else if (IS_PANY(root, LoopStatement)) {
        P_TRANS(node, root, LoopStatement);
        if (node->type == LoopStatement::WHILE) {
            ret << "while# " << endl;
        } else {
            ret << "reapeat# " << endl;
        }
        ret << dfs(node->cond, depth + 1);
        ret << dfs(node->stats, depth + 1);
    } else if (IS_PANY(root, ForeachStatement)) {
        P_TRANS(node, root, ForeachStatement);
        ret << indent << "foreach# element: <" << node->element->name
            << "> array: <" << node->array->name << ">" << endl;
        ret << dfs(node->stats, depth + 1);
    } else {
        
    }
    return ret.str();
    
}
string ast_string(ASTNode *root)
{
    cout << IS_PANY(root, Program) << endl;
    //    return dfs(root, 0);
    return "";
}
