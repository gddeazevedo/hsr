#include <asmjit/x86.h>
#include <iostream>
#include <memory>
#include <vector>

using Func = float(*)(void);

// SSE registers
std::vector<asmjit::x86::Vec> reg_pool = {
    asmjit::x86::xmm0, asmjit::x86::xmm1, asmjit::x86::xmm2,
    asmjit::x86::xmm3, asmjit::x86::xmm4, asmjit::x86::xmm5
};

enum class NodeType {
    Operand, Operator
};

struct TreeNode {
    char value;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
    NodeType type;

    TreeNode(char val, NodeType t) : value(val), type(t) {}
};

struct Tree {
    std::unique_ptr<TreeNode> root;

    Tree(char val, NodeType type) {
        root = std::make_unique<TreeNode>(val, type);
    }

    void walk_in_order(const std::unique_ptr<TreeNode>& node) {
        if (!node) return;
        walk_in_order(node->left);
        std::cout << node->value << " ";
        walk_in_order(node->right);
    }

    void walk_in_order() {
        walk_in_order(root);
        std::cout << std::endl;
    }
};

void generate_assembly_from_tree(
    const std::unique_ptr<TreeNode>& node,
    asmjit::x86::Assembler& a,
    std::vector<asmjit::x86::Vec>& reg_stack
) {
    if (!node) return;

    generate_assembly_from_tree(node->left, a, reg_stack);
    generate_assembly_from_tree(node->right, a, reg_stack);

    if (node->type == NodeType::Operator) {
        asmjit::x86::Vec right = reg_stack.back(); reg_stack.pop_back();
        asmjit::x86::Vec left = reg_stack.back(); reg_stack.pop_back();

        asmjit::x86::Vec result = left;

        switch (node->value) {
            case '+':
                a.vaddps(result, left, right);
                break;
            case '-':
                a.vsubps(result, left, right);
                break;
            case '*':
                a.vmulps(result, left, right);
                break;
            case '/':
                a.vdivps(result, left, right);
                break;
        }

        reg_pool.push_back(right);
        reg_stack.push_back(result);
    } else {
        float operand = static_cast<float>(node->value - '0');
        asmjit::x86::Vec reg = reg_pool.back(); reg_pool.pop_back();

        a.sub(asmjit::x86::rsp, 8);
        a.mov(asmjit::x86::dword_ptr(asmjit::x86::rsp), asmjit::imm(*reinterpret_cast<int*>(&operand)));
        a.movss(reg, asmjit::x86::dword_ptr(asmjit::x86::rsp));
        a.add(asmjit::x86::rsp, 8);

        reg_stack.push_back(reg);
    }
}

int main() {
    // (5 * 4) + (3 / 3) * 3
    auto tree = std::make_unique<Tree>('+', NodeType::Operator);
    tree->root->left = std::make_unique<TreeNode>('*', NodeType::Operator);
    tree->root->left->left = std::make_unique<TreeNode>('5', NodeType::Operand);
    tree->root->left->right = std::make_unique<TreeNode>('4', NodeType::Operand);
    tree->root->right = std::make_unique<TreeNode>('*', NodeType::Operator);
    tree->root->right->left = std::make_unique<TreeNode>('/', NodeType::Operator);
    tree->root->right->left->left = std::make_unique<TreeNode>('3', NodeType::Operand);
    tree->root->right->left->right = std::make_unique<TreeNode>('3', NodeType::Operand);
    tree->root->right->right = std::make_unique<TreeNode>('3', NodeType::Operand);

    tree->walk_in_order();

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpu_features());
    asmjit::x86::Assembler a(&code);

    a.push(asmjit::x86::rbp);
    a.mov(asmjit::x86::rbp, asmjit::x86::rsp);

    std::vector<asmjit::x86::Vec> reg_reg_stack;

    generate_assembly_from_tree(tree->root, a, reg_reg_stack);

    if (reg_reg_stack.back() != asmjit::x86::xmm0)
        a.movaps(asmjit::x86::xmm0, reg_reg_stack.back());

    a.mov(asmjit::x86::rsp, asmjit::x86::rbp);
    a.pop(asmjit::x86::rbp);
    a.ret();

    Func fn;
    asmjit::Error err = rt.add(&fn, &code);
    if ((bool)err) {
        std::cerr << "Erro ao compilar: " << asmjit::DebugUtils::error_as_string(err) << "\n";
        return 1;
    }

    float result = fn();
    std::cout << "Resultado: " << result << std::endl;

    rt.release(fn);
}
