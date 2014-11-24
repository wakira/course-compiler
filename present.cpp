#include "present.h"
#include <iostream>
#include <sstream>
#include <cassert>

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
        return ret.str();
    string indent("");
    for (int i = 0; i < depth; ++i) {
        indent += TAB;
        indent += LIM;
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
        P_TRANS(node, root, ArrayDefinition);
        ret << indent
            << "Array# type: <" << node->type->name
            << "> name: <"
            << node->name->name
            << "> size: " << node->size << endl;
    } else if (IS_PANY(root, ClassDefinition)) {
        P_TRANS(node, root, ClassDefinition);
        ret << indent << "Class# name: <" << node->className->name << flush;
        if (node->extFrom != NULL) {
            ret << "> parent: <" << node->extFrom->name << flush;
        }
        ret << ">: " << endl;
        ret << dfs(node->variables, depth + 1);
        ret << dfs(node->functions, depth + 1);
    } else if (IS_PANY(root, FunctionDefinition)) {
        P_TRANS(node, root, FunctionDefinition);
        ret << indent << "Function# name: <" << node->name->name << flush;
        if (node->retType != NULL) {
            ret << "> return <" << node->retType->name << flush;
        }
        ret << ">: " << endl;
        ret << dfs(node->arguments, depth + 1);
        ret << dfs(node->variables, depth + 1);
        ret << dfs(node->functionBlock, depth + 1);
    } else if (IS_PANY(root, Primary)) {
        P_TRANS(node, root, Primary);
        ret << dfs(node->expr, depth) << flush;
    } else if (IS_PANY(root, IdentPr)) {
        P_TRANS(node, root, IdentPr);
        ret << indent << node->name->name << endl;
    } else if (IS_PANY(root, ArrayPr)) {
        P_TRANS(node, root, ArrayPr);
        ret << dfs(node->name, depth)
            << dfs(node->index, depth + 1);
    } else if (IS_PANY(root, NumericLiteral)) {
        P_TRANS(node, root, NumericLiteral);
        ret << indent << node->val << endl;
    } else if (IS_PANY(root, BinaryOperation)) {
        P_TRANS(node, root, BinaryOperation);
        if (getOp(node->op) != "NOP") {
            ret << indent << getOp(node->op) << endl;
            ret << dfs(node->a, depth + 1);
            ret << dfs(node->b, depth + 1);
        } else {
            if (node->a != NULL) {
                ret << dfs(node->a, depth);
            } else {
                ret << dfs(node->b, depth);
            }
        }
    } else if (IS_PANY(root, UnaryOperation)) {
        P_TRANS(node, root, UnaryOperation);
        if (getOp(node->op) != "NOP") {
            ret << indent << "(Unary)" << getOp(node->op) << endl;
            ret << dfs(node->p, depth + 1);
        } else {
            ret << dfs(node->p, depth);
        }
    } else if (IS_PANY(root, FunCall)) {
        P_TRANS(node, root, FunCall);
        ret << indent << "Call# name: <" << endl
            << dfs(node->name, depth + 1)
            << indent << ">: " << endl;
        ret << dfs(node->args, depth + 1);
    } else if (IS_PANY(root, DotOperation)) {
        P_TRANS(node, root, DotOperation);
        ret << indent << "." << endl;
        ret << dfs(node->pr, depth + 1);
        ret << indent << TAB << LIM << node->field->name << endl;
    } else if (IS_PANY(root, ElementList)) {
        P_TRANS(node, root, ElementList);
        ret << indent << "List: " << endl;
        for (list<ASTNode *>::iterator i = node->elements.begin();
             i != node->elements.end(); ++i) {
            ret << dfs(*i, depth + 1);
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
        assert(node->conds->elements.size() == node->stats.size());
        list<ElementList *>::iterator si = node->stats.begin();
        list<ASTNode *>::iterator ci = node->conds->elements.begin();
        ret << indent << "if#" << endl;
        for (int i = 0; i < node->stats.size(); ++i) {
            ret << indent << TAB << "condition: " << endl;
            if (*ci == NULL) {
                ret << indent << TAB << LIM << "true" << endl;
            } else {
                ret << dfs(*ci, depth + 1);
                ++ci;
            }
            ret << indent << TAB << "Statements:" << endl;
            ret << dfs(*si, depth + 1);
            si++;
        }
    } else if (IS_PANY(root, LoopStatement)) {
        P_TRANS(node, root, LoopStatement);
        if (node->type == LoopStatement::WHILE) {
            ret << indent << "while# " << endl;
        } else {
            ret << indent << "reapeat# " << endl;
        }
        ret << dfs(node->cond, depth + 1);
        ret << dfs(node->stats, depth + 1);
    } else if (IS_PANY(root, ForeachStatement)) {
        P_TRANS(node, root, ForeachStatement);
        ret << indent << "foreach# element: <" << node->element->name
            << "> array: <" << node->array->name << ">" << endl;
        ret << dfs(node->stats, depth + 1);
    } else if (IS_PANY(root, FuncStatement)){
        P_TRANS(node, root, FuncStatement);
        ret << dfs(node->call, depth);
    }
    return ret.str();
    
}
string ast_string(ASTNode *root)
{
    return dfs(root, 0);
}
