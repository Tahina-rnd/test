import { useState } from 'react';
import { Plus, Search, DollarSign, CreditCard, AlertCircle } from 'lucide-react';
import { DashboardLayout } from '@/components/layout/DashboardLayout';
import { PageHeader } from '@/components/common/PageHeader';
import { DataTable } from '@/components/common/DataTable';
import { StatusBadge } from '@/components/common/StatusBadge';
import { StatCard } from '@/components/dashboard/StatCard';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogFooter,
} from '@/components/ui/dialog';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Label } from '@/components/ui/label';
import { useToast } from '@/hooks/use-toast';

interface Fee {
  id: string;
  studentName: string;
  description: string;
  amount: number;
  paidAmount: number;
  dueDate: string;
  status: 'paid' | 'partial' | 'unpaid';
}

interface Payment {
  id: string;
  feeId: string;
  amount: number;
  date: string;
  method: string;
}

// Mock data
const initialFees: Fee[] = [
  { id: '1', studentName: 'Emma Johnson', description: 'Tuition Fee - Term 1', amount: 1500, paidAmount: 1500, dueDate: '2024-01-15', status: 'paid' },
  { id: '2', studentName: 'Liam Smith', description: 'Tuition Fee - Term 1', amount: 1500, paidAmount: 800, dueDate: '2024-01-15', status: 'partial' },
  { id: '3', studentName: 'Olivia Brown', description: 'Tuition Fee - Term 1', amount: 1500, paidAmount: 0, dueDate: '2024-01-20', status: 'unpaid' },
  { id: '4', studentName: 'Noah Williams', description: 'Library Fee', amount: 200, paidAmount: 200, dueDate: '2024-01-10', status: 'paid' },
  { id: '5', studentName: 'Ava Davis', description: 'Tuition Fee - Term 1', amount: 1500, paidAmount: 0, dueDate: '2024-01-25', status: 'unpaid' },
  { id: '6', studentName: 'Sophia Garcia', description: 'Sports Fee', amount: 300, paidAmount: 300, dueDate: '2024-01-12', status: 'paid' },
  { id: '7', studentName: 'Mason Martinez', description: 'Tuition Fee - Term 1', amount: 1500, paidAmount: 500, dueDate: '2024-01-18', status: 'partial' },
  { id: '8', studentName: 'Isabella Lopez', description: 'Lab Fee', amount: 250, paidAmount: 0, dueDate: '2024-01-22', status: 'unpaid' },
];

const students = ['Emma Johnson', 'Liam Smith', 'Olivia Brown', 'Noah Williams', 'Ava Davis', 'Sophia Garcia', 'Mason Martinez', 'Isabella Lopez'];
const paymentMethods = ['Cash', 'Card', 'Bank Transfer', 'Check'];

export default function Fees() {
  const [fees, setFees] = useState<Fee[]>(initialFees);
  const [searchQuery, setSearchQuery] = useState('');
  const [statusFilter, setStatusFilter] = useState<string>('all');
  const [isAddFeeModalOpen, setIsAddFeeModalOpen] = useState(false);
  const [isPaymentModalOpen, setIsPaymentModalOpen] = useState(false);
  const [selectedFee, setSelectedFee] = useState<Fee | null>(null);
  const [feeFormData, setFeeFormData] = useState({
    studentName: '',
    description: '',
    amount: '',
    dueDate: new Date().toISOString().split('T')[0],
  });
  const [paymentFormData, setPaymentFormData] = useState({
    amount: '',
    method: '',
    date: new Date().toISOString().split('T')[0],
  });
  const { toast } = useToast();

  const filteredFees = fees.filter((fee) => {
    const matchesSearch = fee.studentName.toLowerCase().includes(searchQuery.toLowerCase()) ||
      fee.description.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesStatus = statusFilter === 'all' || fee.status === statusFilter;
    return matchesSearch && matchesStatus;
  });

  // Stats
  const totalDue = fees.reduce((acc, f) => acc + f.amount, 0);
  const totalPaid = fees.reduce((acc, f) => acc + f.paidAmount, 0);
  const unpaidCount = fees.filter((f) => f.status !== 'paid').length;

  const handleAddFee = () => {
    if (!feeFormData.studentName || !feeFormData.description || !feeFormData.amount) {
      toast({
        title: 'Error',
        description: 'Please fill in all required fields',
        variant: 'destructive',
      });
      return;
    }

    const newFee: Fee = {
      id: Date.now().toString(),
      studentName: feeFormData.studentName,
      description: feeFormData.description,
      amount: Number(feeFormData.amount),
      paidAmount: 0,
      dueDate: feeFormData.dueDate,
      status: 'unpaid',
    };
    setFees([...fees, newFee]);
    setIsAddFeeModalOpen(false);
    setFeeFormData({ studentName: '', description: '', amount: '', dueDate: new Date().toISOString().split('T')[0] });
    toast({ title: 'Success', description: 'Fee added successfully' });
  };

  const handleRecordPayment = () => {
    if (!selectedFee || !paymentFormData.amount || !paymentFormData.method) {
      toast({
        title: 'Error',
        description: 'Please fill in all required fields',
        variant: 'destructive',
      });
      return;
    }

    const paymentAmount = Number(paymentFormData.amount);
    const newPaidAmount = selectedFee.paidAmount + paymentAmount;
    const newStatus = newPaidAmount >= selectedFee.amount ? 'paid' : 'partial';

    setFees(fees.map((f) =>
      f.id === selectedFee.id
        ? { ...f, paidAmount: newPaidAmount, status: newStatus }
        : f
    ));

    setIsPaymentModalOpen(false);
    setSelectedFee(null);
    setPaymentFormData({ amount: '', method: '', date: new Date().toISOString().split('T')[0] });
    toast({ title: 'Success', description: 'Payment recorded successfully' });
  };

  const openPaymentModal = (fee: Fee) => {
    setSelectedFee(fee);
    setPaymentFormData({
      amount: (fee.amount - fee.paidAmount).toString(),
      method: '',
      date: new Date().toISOString().split('T')[0],
    });
    setIsPaymentModalOpen(true);
  };

  const columns = [
    { key: 'studentName', header: 'Student' },
    { key: 'description', header: 'Description' },
    {
      key: 'amount',
      header: 'Amount',
      render: (fee: Fee) => `$${fee.amount.toLocaleString()}`,
    },
    {
      key: 'paidAmount',
      header: 'Paid',
      render: (fee: Fee) => (
        <span className={fee.paidAmount === fee.amount ? 'text-success' : 'text-foreground'}>
          ${fee.paidAmount.toLocaleString()}
        </span>
      ),
    },
    { key: 'dueDate', header: 'Due Date' },
    {
      key: 'status',
      header: 'Status',
      render: (fee: Fee) => <StatusBadge status={fee.status} />,
    },
    {
      key: 'actions',
      header: '',
      className: 'w-32',
      render: (fee: Fee) => (
        fee.status !== 'paid' && (
          <Button
            variant="outline"
            size="sm"
            onClick={() => openPaymentModal(fee)}
          >
            Record Payment
          </Button>
        )
      ),
    },
  ];

  return (
    <DashboardLayout>
      <PageHeader title="School Fees" subtitle="Manage tuition and fee payments">
        <Button onClick={() => setIsAddFeeModalOpen(true)}>
          <Plus className="w-4 h-4 mr-2" /> Add Fee
        </Button>
      </PageHeader>

      {/* Stats */}
      <div className="grid grid-cols-1 sm:grid-cols-3 gap-6 mb-8">
        <StatCard
          title="Total Due"
          value={`$${totalDue.toLocaleString()}`}
          icon={DollarSign}
          variant="default"
        />
        <StatCard
          title="Total Collected"
          value={`$${totalPaid.toLocaleString()}`}
          subtitle={`${((totalPaid / totalDue) * 100).toFixed(0)}% collected`}
          icon={CreditCard}
          variant="success"
        />
        <StatCard
          title="Pending Payments"
          value={unpaidCount}
          subtitle="Students with outstanding fees"
          icon={AlertCircle}
          variant="warning"
        />
      </div>

      {/* Filters */}
      <div className="flex flex-col sm:flex-row gap-4 mb-6">
        <div className="relative flex-1">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
          <Input
            placeholder="Search by student or description..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="pl-10"
          />
        </div>
        <Select value={statusFilter} onValueChange={setStatusFilter}>
          <SelectTrigger className="w-full sm:w-40">
            <SelectValue placeholder="All Status" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="all">All Status</SelectItem>
            <SelectItem value="paid">Paid</SelectItem>
            <SelectItem value="partial">Partial</SelectItem>
            <SelectItem value="unpaid">Unpaid</SelectItem>
          </SelectContent>
        </Select>
      </div>

      {/* Table */}
      <DataTable
        columns={columns}
        data={filteredFees}
        keyExtractor={(fee) => fee.id}
        emptyMessage="No fees found"
      />

      {/* Add Fee Modal */}
      <Dialog open={isAddFeeModalOpen} onOpenChange={setIsAddFeeModalOpen}>
        <DialogContent className="sm:max-w-lg">
          <DialogHeader>
            <DialogTitle>Add New Fee</DialogTitle>
          </DialogHeader>
          <div className="grid gap-4 py-4">
            <div className="space-y-2">
              <Label>Student *</Label>
              <Select value={feeFormData.studentName} onValueChange={(v) => setFeeFormData({ ...feeFormData, studentName: v })}>
                <SelectTrigger>
                  <SelectValue placeholder="Select student" />
                </SelectTrigger>
                <SelectContent>
                  {students.map((student) => (
                    <SelectItem key={student} value={student}>{student}</SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>
            <div className="space-y-2">
              <Label htmlFor="description">Description *</Label>
              <Input
                id="description"
                placeholder="e.g., Tuition Fee - Term 1"
                value={feeFormData.description}
                onChange={(e) => setFeeFormData({ ...feeFormData, description: e.target.value })}
              />
            </div>
            <div className="grid grid-cols-2 gap-4">
              <div className="space-y-2">
                <Label htmlFor="feeAmount">Amount *</Label>
                <Input
                  id="feeAmount"
                  type="number"
                  placeholder="0.00"
                  value={feeFormData.amount}
                  onChange={(e) => setFeeFormData({ ...feeFormData, amount: e.target.value })}
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="dueDate">Due Date</Label>
                <Input
                  id="dueDate"
                  type="date"
                  value={feeFormData.dueDate}
                  onChange={(e) => setFeeFormData({ ...feeFormData, dueDate: e.target.value })}
                />
              </div>
            </div>
          </div>
          <DialogFooter>
            <Button variant="outline" onClick={() => setIsAddFeeModalOpen(false)}>
              Cancel
            </Button>
            <Button onClick={handleAddFee}>Add Fee</Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>

      {/* Record Payment Modal */}
      <Dialog open={isPaymentModalOpen} onOpenChange={setIsPaymentModalOpen}>
        <DialogContent className="sm:max-w-md">
          <DialogHeader>
            <DialogTitle>Record Payment</DialogTitle>
          </DialogHeader>
          {selectedFee && (
            <div className="py-4">
              <div className="bg-muted/50 rounded-lg p-4 mb-4">
                <p className="font-medium">{selectedFee.studentName}</p>
                <p className="text-sm text-muted-foreground">{selectedFee.description}</p>
                <div className="mt-2 flex justify-between text-sm">
                  <span>Outstanding:</span>
                  <span className="font-medium text-destructive">
                    ${(selectedFee.amount - selectedFee.paidAmount).toLocaleString()}
                  </span>
                </div>
              </div>
              <div className="space-y-4">
                <div className="space-y-2">
                  <Label htmlFor="paymentAmount">Payment Amount *</Label>
                  <Input
                    id="paymentAmount"
                    type="number"
                    value={paymentFormData.amount}
                    onChange={(e) => setPaymentFormData({ ...paymentFormData, amount: e.target.value })}
                  />
                </div>
                <div className="space-y-2">
                  <Label>Payment Method *</Label>
                  <Select value={paymentFormData.method} onValueChange={(v) => setPaymentFormData({ ...paymentFormData, method: v })}>
                    <SelectTrigger>
                      <SelectValue placeholder="Select method" />
                    </SelectTrigger>
                    <SelectContent>
                      {paymentMethods.map((method) => (
                        <SelectItem key={method} value={method}>{method}</SelectItem>
                      ))}
                    </SelectContent>
                  </Select>
                </div>
                <div className="space-y-2">
                  <Label htmlFor="paymentDate">Payment Date</Label>
                  <Input
                    id="paymentDate"
                    type="date"
                    value={paymentFormData.date}
                    onChange={(e) => setPaymentFormData({ ...paymentFormData, date: e.target.value })}
                  />
                </div>
              </div>
            </div>
          )}
          <DialogFooter>
            <Button variant="outline" onClick={() => setIsPaymentModalOpen(false)}>
              Cancel
            </Button>
            <Button onClick={handleRecordPayment}>Record Payment</Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </DashboardLayout>
  );
}
