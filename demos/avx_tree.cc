#include <asmjit/x86.h>
#include <iostream>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdint>

using Func = void(*)(const float*, float*, size_t);

enum class NodeType { Constant, Operator, Input };

struct TreeNode {
    char value;
    std::vector<float> input;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
    NodeType type;

    TreeNode(char val, NodeType t) : value(val), type(t) {}
    TreeNode(const std::vector<float>& in, NodeType t) : value('x'), input(in), type(t) {}
};

struct Tree {
    std::unique_ptr<TreeNode> root;

    Tree(char val, NodeType t) : root(std::make_unique<TreeNode>(val, t)) {}

    private:
        void walk_in_order(const std::unique_ptr<TreeNode>& node) {
            if (!node) return;
            walk_in_order(node->left);
            std::cout << node->value << " ";
            walk_in_order(node->right);
        }

    public:
        void walk_in_order() {
            walk_in_order(root);
            std::cout << std::endl;
        }
};

std::vector<asmjit::x86::Vec> reg_stack = {
    asmjit::x86::ymm0, asmjit::x86::ymm1, asmjit::x86::ymm2, asmjit::x86::ymm3,
    asmjit::x86::ymm4, asmjit::x86::ymm5, asmjit::x86::ymm6, asmjit::x86::ymm7,
    asmjit::x86::ymm8, asmjit::x86::ymm9, asmjit::x86::ymm10, asmjit::x86::ymm11,
    asmjit::x86::ymm12, asmjit::x86::ymm13, asmjit::x86::ymm14, asmjit::x86::ymm15
};

void generate_assembly_from_tree(
    const std::unique_ptr<TreeNode>& node,
    asmjit::x86::Assembler& a,
    std::vector<asmjit::x86::Vec>& local_reg_stack
) {
    if (!node) return;

    generate_assembly_from_tree(node->left, a, local_reg_stack);
    generate_assembly_from_tree(node->right, a, local_reg_stack);

    if (node->type == NodeType::Constant) {
        asmjit::x86::Vec dst = reg_stack.back();
        reg_stack.pop_back();
        float cval = float(node->value - '0');
        uint32_t bits;
        std::memcpy(&bits, &cval, sizeof(bits));


        a.mov(asmjit::x86::eax, asmjit::imm(bits));
        a.movd(asmjit::x86::xmm15, asmjit::x86::eax);
        a.vbroadcastss(dst, asmjit::x86::xmm15);

        float dst_ptr[8];

        a.mov(asmjit::x86::rcx, asmjit::imm((uint64_t)dst_ptr));
        a.vmovups(asmjit::x86::ptr(asmjit::x86::rcx), dst);

        for ( int i = 0; i < 8; i++ ) std::cout << dst_ptr[i] << " ";
        std::cout <<"end";
        std::cout << std::endl;


        // a.vbroadcastss(dst, asmjit::x86::ptr((uint64_t)&bits));

        local_reg_stack.push_back(dst);
    } else if (node->type == NodeType::Input) {
        asmjit::x86::Vec dst = reg_stack.back(); // ymm
        reg_stack.pop_back();
        a.vmovups(dst, asmjit::x86::ptr(asmjit::x86::rdi, 0, 1));
        local_reg_stack.push_back(dst);
    } else if (node->type == NodeType::Operator) {
        asmjit::x86::Vec right = local_reg_stack.back();
        local_reg_stack.pop_back();
        asmjit::x86::Vec left  = local_reg_stack.back();
        local_reg_stack.pop_back();
        asmjit::x86::Vec result = left;

        switch (node->value) {
            case '+': a.vaddps(result, left, right); break;
            case '-': a.vsubps(result, left, right); break;
            case '*': a.vmulps(result, left, right); break;
            case '/': a.vdivps(result, left, right); break;
        }

        a.vmovups(asmjit::x86::ptr(asmjit::x86::rsi), result);
        local_reg_stack.push_back(result);

        reg_stack.push_back(right);
        reg_stack.push_back(left);
    }
}

int main() {
    std::vector<float> inputVec = {1,2,3,4,5,6,7,8};
    std::vector<float> outputVec(8, 0.0f);

    // 5 * x + 3
    auto tree = std::make_unique<Tree>('+', NodeType::Operator);
    tree->root->right = std::make_unique<TreeNode>('3', NodeType::Constant);
    tree->root->left  = std::make_unique<TreeNode>('*', NodeType::Operator);
    tree->root->left->left  = std::make_unique<TreeNode>('5', NodeType::Constant);
    tree->root->left->right = std::make_unique<TreeNode>(inputVec, NodeType::Input);
    tree->walk_in_order();


    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.environment(), rt.cpu_features());
    asmjit::x86::Assembler a(&code);

    std::vector<asmjit::x86::Vec> local_stack;
    generate_assembly_from_tree(tree->root, a, local_stack);

    a.vzeroupper(); // zera registradores YMM
    a.ret();

    Func fn = nullptr;
    rt.add(&fn, &code);

    fn(inputVec.data(), outputVec.data(), inputVec.size());

    for (float v : outputVec) std::cout << v << " ";
    std::cout << std::endl;

    rt.release(fn);
}
