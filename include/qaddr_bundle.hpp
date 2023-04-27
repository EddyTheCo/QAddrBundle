#pragma once
#include"encoding/qbech32.hpp"
#include"client/qclient.hpp"
#include<QString>
#include<QByteArray>
#include<set>
namespace qiota{

using namespace qblocks;

class AddressBundle
{
public:
    AddressBundle(const std::pair<QByteArray,QByteArray>& key_pair_m);

    AddressBundle(const std::shared_ptr<const Address>& addr_m);

    QString get_address_bech32(QString hrp)const;
    std::shared_ptr<const Address> get_address(void)const;


    void consume_outputs(std::vector<Node_output> &outs_, const quint64 amount_need_it=0);


    void create_unlocks(const QByteArray & message,const quint16& ref=0);
    pvector<const Native_Token> get_tokens(const c_array& tokenid="" )const;
    qblocks::c_array Inputs_hash;
    quint64 amount;
    pvector<const Output> ret_outputs;
    pvector<Output> alias_outputs,foundry_outputs, nft_outputs;

    pvector<const Input> inputs;
    std::vector<Output::types> ref_typs;
    pvector<const Unlock> unlocks;
    std::map<qblocks::c_array,quint256> native_tokens;
    std::vector<quint32> to_unlock,to_expire;
    std::set<QString> otids;

private:
    const std::pair<QByteArray,QByteArray> key_pair;
    std::shared_ptr<const Address> addr;


};
using address_bundle = AddressBundle;
}
