#include<QTimer>
#include"qaddr_bundle.hpp"
#include"encoding/qbech32.hpp"
#include"crypto/qed25519.hpp"
#include"nodeConnection.hpp"

namespace qiota{

using namespace qblocks;

AddressBox::AddressBox(const std::pair<QByteArray,QByteArray>& keyPair)
    :m_keyPair(keyPair),
    m_addr(std::shared_ptr<Address>(new Ed25519_Address(
        QCryptographicHash::hash(keyPair.first,QCryptographicHash::Blake2b_256)))),
    m_amount(0)
    {
    };
AddressBox::AddressBox(const std::shared_ptr<const Address>& addr, c_array outId):m_addr(addr),m_amount(0),m_outId(outId)
    {
    };
std::shared_ptr<const Address> AddressBox::getAddress(void)const
{
    return m_addr;
}
QString AddressBox::getAddressHash(void)const
{
    return m_addr->addrhash().toHexString();
}
QString AddressBox::getAddressBech32(const QString hrp)const
{
    const auto addr=qencoding::qbech32::Iota::encode(hrp,m_addr->addr());
    return addr;
}
void AddressBox::monitorToExpire(const c_array outId,const quint32 unixTime)
{
    const auto triger=(unixTime-QDateTime::currentDateTime().toSecsSinceEpoch())*1000;
    QTimer::singleShot(triger,this,[=](){
        std::vector<c_array>  rm_inputs;
        std::vector<c_array>  rm_address;
        rmInput(outId,rm_inputs,rm_address);
    });
}
void AddressBox::monitorToUnlock(const c_array outId, const quint32 unixTime)
{
    const auto triger=(unixTime-QDateTime::currentDateTime().toSecsSinceEpoch())*1000;
    QTimer::singleShot(triger,this,[=](){
        auto resp=NodeConnection::instance()->mqtt()->get_outputs_outputId(outId.toHexString());
        connect(resp,&ResponseMqtt::returned,this,[=](QJsonValue data){
            auto node_outputs=std::vector<Node_output>{Node_output(data)};
            getOutputs({node_outputs});
            resp->deleteLater();
        });
    });
}
void AddressBox::monitorToSpend(const c_array outId)
{

    auto resp=NodeConnection::instance()->mqtt()->get_outputs_outputId(outId.toHexString());
    connect(resp,&ResponseMqtt::returned,this,[=](QJsonValue data){
        const auto node_output=Node_output(data);
        if(node_output.metadata().is_spent_)
        {
            resp->deleteLater();
            std::vector<c_array>  rm_inputs;
            std::vector<c_array>  rm_address;
            rmInput(outId,rm_inputs,rm_address);
        }

    });
}
void AddressBox::addInput(const c_array outId, const InBox & inBox)
{
    m_inputs.insert(outId,inBox);
    emit inputAdded(outId);

    if(inBox.output->type()==Output::NFT_typ||inBox.output->type()==Output::Alias_typ)
    {
        AddressBox* nextAddr=nullptr;
        if(inBox.output->type()==Output::NFT_typ)
        {
            nextAddr=new AddressBox(Address::NFT(inBox.output->get_id()),outId);
        }
        if(inBox.output->type()==Output::Alias_typ)
        {
            nextAddr=new AddressBox(Address::Alias(inBox.output->get_id()),outId);
        }
        addAddrBox(outId,nextAddr);
    }
    setAmount(m_amount+inBox.amount);
}
void AddressBox::rmInput(const c_array outId, std::vector<c_array> & rm_inputs,std::vector<c_array> & rm_addresses)
{
    if(m_inputs.contains(outId))
    {
        const auto input=m_inputs.take(outId);
        const bool isroot=(rm_inputs.size()==0&&rm_addresses.size()==0);
        quint64 amount=input.amount;
        rm_inputs.push_back(outId);
        if(input.output->type()==Output::NFT_typ||input.output->type()==Output::Alias_typ)
        {
            const auto addressBox=m_AddrBoxes.take(outId);
            amount+=addressBox->amount();
            const auto address=addressBox->getAddress()->addr();
            rm_addresses.push_back(address);
            addressBox->clean(rm_inputs,rm_addresses);
        }

        if(isroot)
        {
            emit inputRemoved(rm_inputs);
            emit addrRemoved(rm_addresses);
            setAmount(m_amount-amount);
        }

    }

}
void AddressBox::addAddrBox(const c_array outId,AddressBox* addrBox)
{
    connect(addrBox,&AddressBox::amountChanged,this,[=](const auto prevA,const auto nextA){
        setAmount(m_amount-prevA+nextA);
    });
    m_AddrBoxes.insert(outId,addrBox);
    emit addrAdded(addrBox);
}
void  AddressBox::clean(std::vector<c_array> & rm_inputs,std::vector<c_array> & rm_addresses)
{
    for(const auto &v:m_inputs.keys())
    {
        rmInput(v,rm_inputs,rm_addresses);
    }
    deleteLater();
}
pvector<const Unlock> AddressBox::getUnlocks(const QByteArray & message, const quint16 &ref, const size_t& inputSize)
{
    pvector<const Unlock> unlocks;
    for(size_t i=0;i<inputSize;i++)
    {
        if(m_addr->type()==Address::Ed25519_typ)
        {
            if(unlocks.size())
            {
                unlocks.push_back(Unlock::Reference(ref));
            }
            else
            {
                unlocks.push_back(Unlock::Signature(Signature::Ed25519(m_keyPair.first,qcrypto::qed25519::sign(m_keyPair,message))));
            }
        }
        if(m_addr->type()==qblocks::Address::Alias_typ)
        {
            unlocks.push_back(Unlock::Alias(ref));
        }
        if(m_addr->type()==qblocks::Address::NFT_typ)
        {
            unlocks.push_back(Unlock::NFT(ref));
        }
    }
    return unlocks;
}
void AddressBox::getOutputs(std::vector<Node_output> &outs_, const quint64 amount_need_it, quint16 howMany)
{
    const auto size=outs_.size();
    while(((amount_need_it)?m_amount<amount_need_it:true)&&((howMany>=size)||(!howMany)?!outs_.empty():outs_.size()+howMany>size))
    {
        const auto v=outs_.back();

        if(!v.metadata().is_spent_&&!(m_inputs.contains(v.metadata().outputid_)))
        {
            const auto output_=v.output();

            const auto  stor_unlock=output_->get_unlock_(Unlock_Condition::Storage_Deposit_Return_typ);
            quint64 retAmount=0;
            std::shared_ptr<const Output> retOut=nullptr;
            if(stor_unlock)
            {
                const auto sdruc=std::static_pointer_cast<const Storage_Deposit_Return_Unlock_Condition>(stor_unlock);
                retAmount=sdruc->return_amount();
                const auto ret_address=sdruc->address();
                const auto retUnlcon = Unlock_Condition::Address(ret_address);
                retOut = Output::Basic(retAmount,{retUnlcon});
            }
            const auto cday=QDateTime::currentDateTime().toSecsSinceEpoch();
            const auto time_lock=output_->get_unlock_(Unlock_Condition::Timelock_typ);
            if(time_lock)
            {
                const auto time_lock_cond=std::static_pointer_cast<const Timelock_Unlock_Condition>(time_lock);
                const auto unix_time=time_lock_cond->unix_time();
                if(cday<unix_time)
                {
                    monitorToUnlock(v.metadata().outputid_,unix_time);
                    outs_.pop_back();
                    retOut=nullptr;
                    continue;
                }
            }
            const auto expir=output_->get_unlock_(Unlock_Condition::Expiration_typ);
            if(expir)
            {
                const auto expiration_cond=std::static_pointer_cast<const Expiration_Unlock_Condition>(expir);
                const auto unix_time=expiration_cond->unix_time();
                const auto ret_address=expiration_cond->address();

                const auto  addr_unlock=std::static_pointer_cast<const Address_Unlock_Condition>(output_->get_unlock_(Unlock_Condition::Address_typ));
                if(ret_address->addr()==getAddress()->addr()&&addr_unlock->address()->addr()!=getAddress()->addr())
                {
                    retOut=nullptr;
                    retAmount=0;

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
                        retOut=nullptr;
                        outs_.pop_back();
                        continue;
                    }
                    monitorToExpire(v.metadata().outputid_,unix_time);
                }

            }

            InBox inBox;
            inBox.input=Input::UTXO(v.metadata().transaction_id_,v.metadata().output_index_);

            qblocks::c_array prevOutputSer;
            prevOutputSer.from_object<Output>(*v.output());
            inBox.inputHash=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);

            if(output_->type()!=Output::Basic_typ)
            {
                if(output_->type()!=Output::Foundry_typ&&output_->get_id()==c_array(32,0))
                {
                    output_->set_id(v.metadata().outputid_);
                }
            }
            inBox.output=output_;
            inBox.amount+=output_->amount_-retAmount;
            monitorToSpend(v.metadata().outputid_);
            addInput(v.metadata().outputid_,inBox);

        }
        outs_.pop_back();
    }
}


}
