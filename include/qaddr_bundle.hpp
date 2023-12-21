#pragma once

#include "block/qinputs.hpp"
#include "block/qoutputs.hpp"
#include "block/qunlocks.hpp"
#include "client/qnode_outputs.hpp"

#if defined(USE_QML)
#include<QtQml>
#endif

#include <QObject>
#include <QtCore/QtGlobal>
#if defined(WINDOWS_QADDR)
# define QADDR_EXPORT Q_DECL_EXPORT
#else
#define QADDR_EXPORT Q_DECL_IMPORT
#endif

namespace qiota{

using namespace qblocks;

#if defined(USE_QML)
class QADDR_EXPORT Qml64:public QObject
{
    quint64 m_value;
    QJsonObject m_json;
    Q_OBJECT

    Q_PROPERTY(QJsonObject  json READ json  NOTIFY jsonChanged)
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    Qml64(const quint64 value=0,QObject *parent = nullptr);
    QJsonObject json(void)const{return m_json;};
    void setValue(const quint64 value);
    const auto& getValue()const{return m_value;};
signals:
    void jsonChanged();
private:
    void intToJSON();
};
#endif
struct InBox {
    std::shared_ptr<const Input> input=nullptr;
    std::shared_ptr<const Output> retOutput=nullptr;
    qblocks::c_array inputHash;
    std::shared_ptr<Output> output=nullptr;
    quint64 amount=0;
};
class QADDR_EXPORT AddressBox:public QObject
{
    Q_OBJECT
#if defined(USE_QML)
    Q_PROPERTY(Qml64*  amount READ amountJson CONSTANT)
    QML_ELEMENT
    QML_UNCREATABLE("")
#endif

public:
    AddressBox(const std::pair<QByteArray,QByteArray>& keyPair,const QString hrp="rms");

    AddressBox(const std::shared_ptr<const Address>& addr,c_array outId=c_array(),const QString hrp="rms");

    std::shared_ptr<const Address> getAddress(void)const;
    Q_INVOKABLE QString getAddressBech32()const;
    QString getAddressHash(void)const;

    void getOutputs(std::vector<Node_output> &outs, const quint64 amountNeedIt=0, const quint16 howMany=0);
    pvector<const Unlock> getUnlocks(const QByteArray & message, const quint16 &ref, const size_t &inputSize);
    quint64 amount(void)const{return m_amount;};
#if defined(USE_QML)
    Qml64* amountJson()const{return m_amountJson;}
#endif
    const auto& inputs(void)const{return m_inputs;};
    c_array outId()const{return m_outId;}

signals:
    void amountChanged(quint64 prevA,quint64 nextA);
    void changed();
    void inputRemoved(std::vector<c_array>);
    void inputAdded(c_array);
    void addrAdded(AddressBox * );
    void addrRemoved(std::vector<c_array>);

private:
    void clean(std::vector<c_array> &rm_inputs, std::vector<c_array> &rm_addresses);
    void monitorToSpend(const c_array outId);
    void monitorToExpire(const c_array outId, const quint32 unixTime);
    void monitorToUnlock(const c_array outId,const quint32 unixTime);
    void rmInput(const c_array outId,std::vector<c_array> & rm_inputs,std::vector<c_array> & rm_addresses);
    void addInput(const c_array outId, const InBox &inBox);
    void addAddrBox(const c_array outId, AddressBox* addrBox);

    void setAmount(const quint64 amount){
        if(amount!=m_amount){const auto oldAmount=m_amount;m_amount=amount;emit amountChanged(oldAmount,amount);}}

    QHash<c_array,InBox> m_inputs;
    QHash<c_array,AddressBox *> m_AddrBoxes;
    quint64 m_amount;
#if defined(USE_QML)
    Qml64* m_amountJson;
#endif
    const std::pair<QByteArray,QByteArray> m_keyPair;
    std::shared_ptr<const Address> m_addr;
    c_array m_outId;
    QString m_hrp;

};
};
