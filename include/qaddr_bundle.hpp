#pragma once

#include"block/qinputs.hpp"
#include"block/qoutputs.hpp"
#include"client/qnode_outputs.hpp"

#include <QObject>
#include <QtCore/QtGlobal>
#if defined(WINDOWS_QWALLET)
# define QWALLET_EXPORT Q_DECL_EXPORT
#else
#define QWALLET_EXPORT Q_DECL_IMPORT
#endif

namespace qiota{

using namespace qblocks;


struct InBox {
    std::shared_ptr<const Input> input=nullptr;
    std::shared_ptr<const Output> retOutput=nullptr;
    qblocks::c_array inputHash;
    std::shared_ptr<Output> output=nullptr;
    quint64 amount=0;
};
class QWALLET_EXPORT AddressBox:public QObject
{
    Q_OBJECT

public:
    AddressBox(const std::pair<QByteArray,QByteArray>& keyPair,QObject *parent = nullptr);

    AddressBox(const std::shared_ptr<const Address>& addr,QObject *parent = nullptr, c_array outId=c_array());

    std::shared_ptr<const Address> getAddress(void)const;
    QString getAddressBech32(const QString hrp)const;
    QString getAddressHash(void)const;

    void getOutputs(std::vector<Node_output> &outs, const quint64 amountNeedIt=0, const quint16 howMany=0);
    pvector<const Unlock> getUnlocks(const QByteArray & message, const quint16 &ref, const size_t &inputSize);
    quint64 amount(void)const{return m_amount;};
    auto addrBoxes(void)const{return m_AddrBoxes;};
    const auto inputs(void)const{return m_inputs;};
    c_array outId()const{return m_outId;}

signals:
    void amountChanged(quint64 prevA,quint64 nextA);
    void changed();
    void inputRemoved(c_array);
    void inputAdded(c_array);
    void addrAdded(AddressBox*);
    void addrRemoved(c_array);

private:
    void clean();
    void monitorToSpend(const c_array outId);
    void monitorToExpire(const c_array outId, const quint32 unixTime);
    void monitorToUnlock(const c_array outId,const quint32 unixTime);
    void rmInput(const c_array outId);
    void addInput(const c_array outId, const InBox inBox);
    void rmAddrBox(const c_array outId, const quint64 outputAmount);
    void addAddrBox(const c_array outId, AddressBox* addrBox);

    void setAmount(const quint64 amount){
        if(amount!=m_amount){const auto oldAmount=m_amount;m_amount=amount;emit amountChanged(oldAmount,amount);}}

    QHash<c_array,InBox> m_inputs;
    QHash<c_array,AddressBox*> m_AddrBoxes;
    quint64 m_amount;

    const std::pair<QByteArray,QByteArray> m_keyPair;
    std::shared_ptr<const Address> m_addr;
    c_array m_outId;


};
};
