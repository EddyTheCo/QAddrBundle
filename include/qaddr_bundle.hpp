#pragma once
#include"encoding/qbech32.hpp"
#include"client/qclient.hpp"
#include<QString>
#include<QByteArray>

namespace qiota{

class AddressBundle
{
public:
    AddressBundle(const std::pair<QByteArray,QByteArray>& key_pair_m);

    AddressBundle(std::shared_ptr<qblocks::Address> addr_m);

    QString get_address_bech32(QString hrp)const;
    std::shared_ptr<qblocks::Address> get_address(void)const;


    void consume_outputs(std::vector<Node_output> &outs_, const quint64 amount_need_it);
    qblocks::signature sign(const QByteArray & message)const;
    std::shared_ptr<qblocks::Signature> signature(const QByteArray & message)const;
    std::shared_ptr<qblocks::Unlock> signature_unlock(const QByteArray & message)const;
    void create_unlocks(const QByteArray & message);

    qblocks::c_array Inputs_Commitments;
    quint64 amount;
    std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> alias_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> foundry_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> nft_outputs;
    std::vector<std::shared_ptr<qblocks::Input>> inputs;
    std::vector<qiota::qblocks::Output::types> ref_typs;
    std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
    std::map<QString,qiota::qblocks::quint256> native_tokens;

private:
    const std::pair<QByteArray,QByteArray> key_pair;
    std::shared_ptr<qblocks::Address> addr;


};
using address_bundle = AddressBundle;
}
