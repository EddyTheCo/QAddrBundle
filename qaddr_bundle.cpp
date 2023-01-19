
#include"qaddr_bundle.hpp"
#include <QCryptographicHash>
#include"crypto/qed25519.hpp"
#include<QDataStream>
#include<QDateTime>
using namespace qcrypto;
namespace qiota{

AddressBundle::AddressBundle(const std::pair<QByteArray,QByteArray>& key_pair_m,QString hrp_m):key_pair(key_pair_m),
    hrp(hrp_m), reference_count_(0)
{ };


QByteArray AddressBundle::get_hash(void)const
{
    return QCryptographicHash::hash(key_pair.first,QCryptographicHash::Blake2b_256);
}

template<qblocks::Address::types addressType> QString AddressBundle::get_address(void)const
{
    QByteArray hash_;
    auto buffer=QDataStream(&hash_,QIODevice::WriteOnly | QIODevice::Append);
    buffer.setByteOrder(QDataStream::LittleEndian);
    buffer<<addressType;
    hash_.append(get_hash());
    const auto addr=qencoding::qbech32::Iota::encode(hrp,hash_);
    return addr;
}

template QString AddressBundle::get_address<qblocks::Address::Ed25519_typ>(void)const;
template QString AddressBundle::get_address<qblocks::Address::Alias_typ>(void)const;
template QString AddressBundle::get_address<qblocks::Address::NFT_typ>(void)const;

std::pair<QByteArray,QByteArray> AddressBundle::get_key_pair(void)const
{
    return key_pair;
}
qblocks::signature AddressBundle::sign(const QByteArray & message)const
{
    return qblocks::signature(qed25519::sign(key_pair,message));
}
std::shared_ptr<qblocks::Signature> AddressBundle::signature(const QByteArray & message)const
{
    return std::shared_ptr<qblocks::Signature>
            (new qblocks::Ed25519_Signature(qblocks::public_key(key_pair.first),
                                            sign(message)));
}
std::shared_ptr<qblocks::Unlock> AddressBundle::signature_unlock(const QByteArray & message)const
{
    return std::shared_ptr<qblocks::Unlock>(new qblocks::Signature_Unlock(signature(message)));
}

void AddressBundle::create_unlocks(const QByteArray & message,std::vector<std::shared_ptr<qblocks::Unlock>>& unlocks)const
{
    if(reference_count_)
    {
        const quint16 reference_index=unlocks.size();
        unlocks.push_back(signature_unlock(message));
        for(auto i=0;i<reference_count_-1;i++)
        {
            unlocks.push_back(std::shared_ptr<qblocks::Unlock>(new qblocks::Reference_Unlock(reference_index)));
        }
    }
}
void AddressBundle::consume_outputs(std::vector<Node_output> outs_,const quint64 amount_need_it,
                                    qblocks::c_array& Inputs_Commitments, quint64& amount,
                                    std::vector<std::shared_ptr<qblocks::Output>>& ret_outputs,
                                    std::vector<std::shared_ptr<qblocks::Input>>& inputs)
{

    const auto cday=QDateTime::currentDateTime().toSecsSinceEpoch();

    while(((amount_need_it)?amount<amount_need_it:1)&&!outs_.empty())
    {
        const auto v=outs_.back();
        if(!v.metadata().is_spent_&&v.output()->type_m==qblocks::Output::Basic_typ)
        {
            const auto basic_output_=std::dynamic_pointer_cast<qblocks::Basic_Output>(v.output());


            const auto  stor_unlock=basic_output_->get_unlock_(qblocks::Unlock_Condition::Storage_Deposit_Return_typ);
            quint64 ret_amount=0;
            if(stor_unlock)
            {
                const auto sdruc=std::dynamic_pointer_cast<qblocks::Storage_Deposit_Return_Unlock_Condition>(stor_unlock);
                ret_amount=sdruc->return_amount();
                const auto ret_address=sdruc->return_address();
                const auto retUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new qblocks::Address_Unlock_Condition(ret_address));
                const auto retOut= std::shared_ptr<qblocks::Output>(new qblocks::Basic_Output(ret_amount,{retUnlcon},{},{}));
                ret_outputs.push_back(retOut);
            }
            const auto expir=basic_output_->get_unlock_(qblocks::Unlock_Condition::Expiration_typ);
            if(expir)
            {
                const auto expiration_cond=std::dynamic_pointer_cast<qblocks::Expiration_Unlock_Condition>(expir);
                const auto unix_time=expiration_cond->unix_time();
                const auto ret_address=expiration_cond->return_address();
                if(ret_address->type_m==0)
                {
                    const auto ret_addrs=std::dynamic_pointer_cast<qblocks::Ed25519_Address>(ret_address);
                    if(ret_addrs->pubkeyhash()==get_hash())
                    {
                        if(stor_unlock)
                        {
                            ret_outputs.pop_back();
                            ret_amount=0;
                        }
                        if(cday<=unix_time)
                        {
                            outs_.pop_back();
                            continue;
                        }
                    }
                    else
                    {
                        if(cday>unix_time)
                        {
                            outs_.pop_back();
                            continue;
                        }
                    }
                }

            }
            const auto time_lock=basic_output_->get_unlock_(qblocks::Unlock_Condition::Timelock_typ);
            if(time_lock)
            {
                const auto time_lock_cond=std::dynamic_pointer_cast<qblocks::Timelock_Unlock_Condition>(time_lock);
                const auto unix_time=time_lock_cond->unix_time();
                if(cday<unix_time)
                {
                    outs_.pop_back();
                    continue;
                }

            }

            inputs.push_back(std::shared_ptr<qblocks::Input>(new qblocks::UTXO_Input(v.metadata().transaction_id_,
                                                                                     v.metadata().output_index_)));
            qblocks::c_array prevOutputSer;
            prevOutputSer.from_object<qblocks::Output>(*(v.output()));
            auto Inputs_Commitment1=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);
            Inputs_Commitments.append(Inputs_Commitment1);
            amount+=basic_output_->amount_-ret_amount;
            reference_count_++;
        }
        outs_.pop_back();
    }
}
}
