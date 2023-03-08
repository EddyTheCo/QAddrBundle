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

    QByteArray get_hash(void)const;
    template<qblocks::Address::types addressType> QString get_address_bech32(QString hrp)const;
    template<qblocks::Address::types addressType> qblocks::c_array get_address(void)const;
    std::pair<QByteArray,QByteArray> get_key_pair(void)const;

    void consume_outputs(std::vector<Node_output> &outs_, const quint64 amount_need_it,
                         qblocks::c_array& Inputs_Commitments, quint64& amount,
                         std::vector<std::shared_ptr<qblocks::Output>>& ret_outputs,
                         std::vector<std::shared_ptr<qblocks::Input>>& inputs);
    quint16 reference_count(void)const{return reference_count_;}
    qblocks::signature sign(const QByteArray & message)const;
    std::shared_ptr<qblocks::Signature> signature(const QByteArray & message)const;
    std::shared_ptr<qblocks::Unlock> signature_unlock(const QByteArray & message)const;
    template<class reference_type>void create_unlocks(const QByteArray & message,std::vector<std::shared_ptr<qblocks::Unlock>>& unlocks)const;

private:
    quint16 reference_count_;
    const std::pair<QByteArray,QByteArray> key_pair;
};
using address_bundle = AddressBundle;
}
