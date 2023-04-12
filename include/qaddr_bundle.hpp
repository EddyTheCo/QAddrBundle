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
    void create_unlocks(const QByteArray & message,const quint16& ref=0);
    std::vector<std::shared_ptr<qblocks::Native_Token>> get_tokens(qblocks::c_array tokenid="" )const;
    qblocks::c_array Inputs_hash;
    quint64 amount;
    std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> alias_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> foundry_outputs;
    std::vector<std::shared_ptr<qblocks::Output>> nft_outputs;
    std::vector<std::shared_ptr<qblocks::Input>> inputs;
    std::vector<qiota::qblocks::Output::types> ref_typs;
    std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
    std::map<qblocks::c_array,quint256> native_tokens;

private:
    const std::pair<QByteArray,QByteArray> key_pair;
    std::shared_ptr<qblocks::Address> addr;


};
using address_bundle = AddressBundle;
}
