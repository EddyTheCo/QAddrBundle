#include<QTimer>
#include"qaddr_bundle.hpp"
#include"encoding/qbech32.hpp"
#include"crypto/qed25519.hpp"
#include"nodeConnection.hpp"

namespace qiota{

using namespace qblocks;

quint64 AddressBox::amount(void)const{return m_amount;};
const QHash<c_array,InBox>& AddressBox::inputs(void)const{return m_inputs;};
c_array AddressBox::outId()const{return m_outId;}
void AddressBox::setAmount(const quint64 amount){
    if(amount!=m_amount){const auto oldAmount=m_amount;m_amount=amount;emit amountChanged(oldAmount,amount);}}

#if defined(USE_QML)
Qml64* AddressBox::amountJson()const{return m_amountJson;}
QJsonObject Qml64::json(void)const{return m_json;};
quint64 Qml64::getValue()const{return m_value;};
void AddressChecker::setValid(bool valid){if(valid!=m_valid){m_valid=valid; emit validChanged();}}
bool AddressChecker::isValid(void)const{return m_valid;};
AddressChecker::AddressChecker(QObject *parent):QObject(parent),m_valid(false)
{
    connect(this, &AddressChecker::addressChanged,this,[=](){
        const auto addr_pair=qencoding::qbech32::Iota::decode(m_address);
        if(addr_pair.second.size())
        {
            setValid(true);
        }
        else
        {
            setValid(false);
        }
    });
};
#endif
QString AddressBox::getAddressBech32()const{return m_bech32address; }
AddressBox::AddressBox(const std::pair<QByteArray,QByteArray>& keyPair,
                       const QString hrp, QObject *parent):QObject(parent),m_keyPair(keyPair),
    m_addr(std::shared_ptr<Address>(new Ed25519_Address(
        QCryptographicHash::hash(keyPair.first,QCryptographicHash::Blake2b_256)))),
    m_amount(0),m_bech32address(qencoding::qbech32::Iota::encode(hrp,m_addr->addr())),m_hrp(hrp)
#if defined(USE_QML)
    ,m_amountJson(new Qml64(m_amount,this))
#endif
{
#if defined(USE_QML)
    connect(this, &AddressBox::amountChanged, m_amountJson,[=](quint64 prevA,quint64 nextA)
            {m_amountJson->setValue(nextA);});
#endif

};
AddressBox::AddressBox(const std::shared_ptr<const Address>& addr, c_array outId,const QString hrp,QObject *parent):QObject(parent),
    m_addr(addr),m_amount(0),m_outId(outId),m_bech32address(qencoding::qbech32::Iota::encode(hrp,m_addr->addr()))
,m_hrp(hrp)
#if defined(USE_QML)
    ,m_amountJson(new Qml64(m_amount,this))
#endif
{
#if defined(USE_QML)
    connect(this, &AddressBox::amountChanged, m_amountJson,[=](quint64 prevA,quint64 nextA)
            {m_amountJson->setValue(nextA);});
#endif
};
std::shared_ptr<const Address> AddressBox::getAddress(void)const
{
    return m_addr;
}
QString AddressBox::getAddressHash(void)const
{
    return m_addr->addrhash().toHexString();
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
void AddressBox::monitorFromExpire(const c_array outId,const quint32 unixTime)
{
    const auto triger=(unixTime-QDateTime::currentDateTime().toSecsSinceEpoch())*1000;
    QTimer::singleShot(triger,this,[=](){

        auto resp=NodeConnection::instance()->rest()->get_api_core_v2_outputs_outputId(outId.toHexString());
        connect(resp,&Response::returned,this,[=](QJsonValue data){
            auto node_outputs=std::vector<Node_output>{Node_output(data)};
            getOutputs({node_outputs});
            resp->deleteLater();
        });
    });
}
void AddressBox::monitorToUnlock(const c_array outId, const quint32 unixTime)
{
    const auto triger=(unixTime-QDateTime::currentDateTime().toSecsSinceEpoch())*1000;
    QTimer::singleShot(triger,this,[=](){
        auto resp=NodeConnection::instance()->rest()->get_api_core_v2_outputs_outputId(outId.toHexString());
        connect(resp,&Response::returned,this,[=](QJsonValue data){
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
            nextAddr=new AddressBox(Address::NFT(inBox.output->get_id()),outId,m_hrp,this);
        }
        if(inBox.output->type()==Output::Alias_typ)
        {
            nextAddr=new AddressBox(Address::Alias(inBox.output->get_id()),outId,m_hrp,this);
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
    delete this;
}
pvector<const Unlock> AddressBox::getUnlocks(const QByteArray & message, const quint16 &ref, const size_t& inputSize) const
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
                if(ret_address->addr()!=addr_unlock->address()->addr())
                {
                    if(ret_address->addr()==getAddress()->addr()&&addr_unlock->address()->addr()!=getAddress()->addr())
                    {
                        retOut=nullptr;
                        retAmount=0;
                        if(cday<=unix_time)
                        {
                            outs_.pop_back();
                            monitorFromExpire(v.metadata().outputid_,unix_time);
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
            }
            InBox inBox;
            inBox.input=Input::UTXO(v.metadata().transaction_id_,v.metadata().output_index_);
            inBox.retOutput=retOut;


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
#if defined(USE_QML)
Qml64::Qml64(const quint64 value,QObject *parent):QObject(parent),m_value(value)
{
    connect(NodeConnection::instance(),&NodeConnection::stateChanged,this,&Qml64::intToJSON);
    intToJSON();
}
void Qml64::setValue(const quint64 value)
{
    if(value!=m_value)
    {
        m_value=value;
        intToJSON();
    }
}
void Qml64::intToJSON()
{
    if(NodeConnection::instance()->state())
    {
        auto info=NodeConnection::instance()->rest()->get_api_core_v2_info();
        connect(info,&Node_info::finished,this,[=]( ){
            QJsonObject shortValue;
            shortValue.insert("unit",info->unit);
            shortValue.insert("value",QString::number(m_value*1.0/std::pow(10,info->decimals),'g', 5));

            QJsonObject largeValue;
            largeValue.insert("unit",info->subunit);
            largeValue.insert("value",QString::number(m_value));
            QJsonObject var;
            var.insert("shortValue",shortValue);
            var.insert("largeValue",largeValue);

            if(m_value>std::pow(10,info->decimals*0.8))
            {
                var.insert("default",0);
            }
            else
            {
                var.insert("default",1);
            }
            m_json=var;
            emit jsonChanged();
            info->deleteLater();
        });
    }
}
#endif

}
