#include<QJsonDocument>
#include"qaddr_bundle.hpp"
#include <QCryptographicHash>
#include"crypto/qed25519.hpp"
#include<QDataStream>
#include<QDateTime>
#include<map>
using namespace qcrypto;
namespace qiota{

using namespace qblocks;

AddressBundle::AddressBundle(const std::pair<QByteArray,QByteArray>& key_pair_m):key_pair(key_pair_m),
    addr(std::shared_ptr<Address>(new Ed25519_Address(QCryptographicHash::hash(key_pair.first,QCryptographicHash::Blake2b_256)))),
    amount(0)
{ };
AddressBundle::AddressBundle(const std::shared_ptr<const Address>& addr_m):addr(addr_m),amount(0)
{ };


std::shared_ptr<const Address> AddressBundle::get_address(void)const
{
    return addr;
}
QString AddressBundle::get_address_bech32(QString hrp)const
{
    const auto addr=qencoding::qbech32::Iota::encode(hrp,get_address()->addr());
    return addr;
}
pvector<const Native_Token> AddressBundle::get_tokens(const c_array &tokenid )const
{
    pvector<const Native_Token> var;
    if(tokenid!="")
    {
        auto search = native_tokens.find(tokenid);
        if(search != native_tokens.end())
        {
            var.push_back(Native_Token::Native(search->first,search->second));
        }
    }
    else
    {
        for (const auto& v : native_tokens)
        {
            var.push_back(Native_Token::Native(v.first,v.second));
        }
    }
    return var;
}
void AddressBundle::create_unlocks(const QByteArray & message, const quint16 &ref)
{
    for(const auto& v:inputs)
    {
        if(addr->type()==Address::Ed25519_typ)
        {
            if(unlocks.size())
            {
                unlocks.push_back(Unlock::Reference(ref));
            }
            else
            {
                unlocks.push_back(Unlock::Signature(Signature::Ed25519(key_pair.first,qed25519::sign(key_pair,message))));
            }
        }
        if(addr->type()==qblocks::Address::Alias_typ)
        {
            unlocks.push_back(std::shared_ptr<qblocks::Unlock>(new qblocks::Alias_Unlock(ref)));
        }
        if(addr->type()==qblocks::Address::NFT_typ)
        {
            unlocks.push_back(std::shared_ptr<qblocks::Unlock>(new qblocks::NFT_Unlock(ref)));
        }
    }

}
void AddressBundle::consume_outputs(std::vector<Node_output> &outs_, const quint64 amount_need_it, quint16 howMany)
{

    const auto size=outs_.size();

    while(((amount_need_it)?amount<amount_need_it:true)&&((howMany>=size)||(!howMany)?!outs_.empty():outs_.size()+howMany>size))
    {
        const auto v=outs_.back();

        if(!v.metadata().is_spent_&&otids.insert(v.metadata().outputid_).second)
        {

            const auto output_=v.output();

            const auto  stor_unlock=output_->get_unlock_(Unlock_Condition::Storage_Deposit_Return_typ);
            quint64 ret_amount=0;
            if(stor_unlock)
            {
                const auto sdruc=std::static_pointer_cast<const Storage_Deposit_Return_Unlock_Condition>(stor_unlock);
                ret_amount=sdruc->return_amount();
                const auto ret_address=sdruc->address();
                const auto retUnlcon = Unlock_Condition::Address(ret_address);
                const auto retOut = Output::Basic(ret_amount,{retUnlcon});
                ret_outputs.push_back(retOut);
            }
            const auto cday=QDateTime::currentDateTime().toSecsSinceEpoch();
            const auto time_lock=output_->get_unlock_(Unlock_Condition::Timelock_typ);
            if(time_lock)
            {
                const auto time_lock_cond=std::static_pointer_cast<const Timelock_Unlock_Condition>(time_lock);
                const auto unix_time=time_lock_cond->unix_time();
                if(cday<unix_time)
                {
                    to_unlock.push_back(unix_time);
                    outs_.pop_back();
                    if(stor_unlock)ret_outputs.pop_back();
                    continue;
                }
            }
            const auto expir=output_->get_unlock_(Unlock_Condition::Expiration_typ);
            if(expir)
            {
                const auto expiration_cond=std::static_pointer_cast<const Expiration_Unlock_Condition>(expir);
                const auto unix_time=expiration_cond->unix_time();
                const auto ret_address=expiration_cond->address();

                if(ret_address->addr()==get_address()->addr())
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
                        if(stor_unlock)ret_outputs.pop_back();
                        outs_.pop_back();
                        continue;
                    }
                    to_expire.push_back(unix_time);
                }

            }


            for(const auto& v:output_->native_tokens_)
            {
                native_tokens[v->token_id()]+=v->amount();
            }

            inputs.push_back(Input::UTXO(v.metadata().transaction_id_,
                                         v.metadata().output_index_));


            qblocks::c_array prevOutputSer;
            prevOutputSer.from_object<Output>(*v.output());
            auto Input_hash=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);
            Inputs_hash+=Input_hash;
            if(output_->type()!=Output::Basic_typ)
            {
                if(output_->type()!=Output::Foundry_typ&&output_->get_id()==c_array(32,0))
                {
                    output_->set_id(v.metadata().outputid_);
                }
                output_->consume();
                if(output_->type()==Output::Foundry_typ)foundry_outputs.push_back(output_);
                if(output_->type()==Output::Alias_typ)alias_outputs.push_back(output_);
                if(output_->type()==Output::NFT_typ)nft_outputs.push_back(output_);
            }
            amount+=output_->amount_-ret_amount;
        }
        outs_.pop_back();
    }
}
}
