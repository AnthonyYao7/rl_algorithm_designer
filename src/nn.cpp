#include <torch/torch.h>
#include <rl_environment/CodingEnvironment.h>

struct TransformerModel : torch::nn::Module {
    torch::nn::TransformerEncoder encoder{nullptr};
    torch::nn::TransformerDecoder decoder{nullptr};

    TransformerModel(int64_t d_model, int64_t nhead, int64_t num_layers) {
        // Encoder Layer
        auto encoder_layer = torch::nn::TransformerEncoderLayer(d_model, nhead);
        encoder = register_module("encoder", torch::nn::TransformerEncoder(encoder_layer, num_layers));

        // Decoder Layer
        auto decoder_layer = torch::nn::TransformerDecoderLayer(d_model, nhead);
        decoder = register_module("decoder", torch::nn::TransformerDecoder(decoder_layer, num_layers));
    }

    torch::Tensor forward(torch::Tensor src, torch::Tensor tgt, torch::Tensor src_mask, torch::Tensor tgt_mask, torch::Tensor memory_mask) {
        torch::Tensor memory = encoder(src, src_mask);
        torch::Tensor output = decoder(tgt, memory, tgt_mask, memory_mask);
        return output;
    }
};

struct ActorCriticAgent {
    TransformerModel actor;
    TransformerModel critic;
    torch::nn::Linear affine{nullptr};

    ActorCriticAgent() : actor(100, 3, 6), critic(100, 3, 6) {
        affine = register_module("affine1", torch::nn::Linear(input_size, hidden_size));
    }

    std::pair<torch::Tensor, torch::Tensor> forward(torch::Tensor state) {
        x = torch::relu(affine1->forward(x));
        auto action_prob = torch::softmax(actor->forward(x), -1);
        auto state_values = critic->forward(x);

        return std::make_pair(action_prob, state_values);
    }

    int select_action(torch::Tensor state) {
        auto forward_result = forward(state);
        auto probs = forward_result.first;
        auto state_value = forward_result.second;
        auto m = torch::distributions::Categorical(probs);
        auto action = m.sample();
      
        return action.item<int>();
    }
};

int main() {
    ActorCriticAgent agent;
    CodingEnvironment env(
            "../../viz/c_grammar_subset.y",
            "../../viz/ansi.c.grammar.l",
            10000,
            "main",
            "LONG",
            {"LONG[]", "LONG"},
            reward_function);

    int input_size = ;
    int hidden_size = /* appropriate hidden size */;
    int num_actions = env.get_action_space().second; // Assuming this returns the number of possible actions
    ActorCriticAgent agent(input_size, hidden_size, num_actions);
  
    for (int episode = 0; episode < 1000; ++episode) {
        
    }
}
