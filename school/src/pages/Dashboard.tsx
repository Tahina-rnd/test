import { Users, BookOpen, CalendarCheck, AlertCircle } from 'lucide-react';
import { DashboardLayout } from '@/components/layout/DashboardLayout';
import { StatCard } from '@/components/dashboard/StatCard';
import { PageHeader } from '@/components/common/PageHeader';
import { DataTable } from '@/components/common/DataTable';
import { StatusBadge } from '@/components/common/StatusBadge';

// Mock data for demo
const mockStats = {
  totalStudents: 1247,
  totalClasses: 42,
  unpaidFees: 156,
  attendanceToday: { present: 1089, absent: 158, total: 1247 },
};

const recentStudents = [
  { id: '1', name: 'Emma Johnson', class: '10-A', status: 'active' as const, enrollmentDate: '2024-01-15' },
  { id: '2', name: 'Liam Smith', class: '9-B', status: 'active' as const, enrollmentDate: '2024-01-14' },
  { id: '3', name: 'Olivia Brown', class: '11-A', status: 'active' as const, enrollmentDate: '2024-01-12' },
  { id: '4', name: 'Noah Williams', class: '8-C', status: 'inactive' as const, enrollmentDate: '2024-01-10' },
  { id: '5', name: 'Ava Davis', class: '12-A', status: 'active' as const, enrollmentDate: '2024-01-08' },
];

const upcomingFees = [
  { id: '1', student: 'Emma Johnson', amount: 1500, dueDate: '2024-02-01', status: 'unpaid' as const },
  { id: '2', student: 'Liam Smith', amount: 1200, dueDate: '2024-02-03', status: 'partial' as const },
  { id: '3', student: 'Sophia Garcia', amount: 1500, dueDate: '2024-02-05', status: 'unpaid' as const },
  { id: '4', student: 'Mason Martinez', amount: 800, dueDate: '2024-02-07', status: 'unpaid' as const },
];

const studentColumns = [
  { key: 'name', header: 'Student Name' },
  { key: 'class', header: 'Class' },
  { 
    key: 'status', 
    header: 'Status',
    render: (item: typeof recentStudents[0]) => <StatusBadge status={item.status} />
  },
  { key: 'enrollmentDate', header: 'Enrolled' },
];

const feeColumns = [
  { key: 'student', header: 'Student' },
  { 
    key: 'amount', 
    header: 'Amount',
    render: (item: typeof upcomingFees[0]) => `$${item.amount.toLocaleString()}`
  },
  { key: 'dueDate', header: 'Due Date' },
  { 
    key: 'status', 
    header: 'Status',
    render: (item: typeof upcomingFees[0]) => <StatusBadge status={item.status} />
  },
];

export default function Dashboard() {
  return (
    <DashboardLayout>
      <PageHeader 
        title="Dashboard" 
        subtitle="Welcome back! Here's an overview of your school."
      />

      {/* Stats Grid */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
        <StatCard
          title="Total Students"
          value={mockStats.totalStudents.toLocaleString()}
          subtitle="Across all classes"
          icon={Users}
          variant="primary"
          trend={{ value: 5.2, isPositive: true }}
        />
        <StatCard
          title="Total Classes"
          value={mockStats.totalClasses}
          subtitle="Active classes"
          icon={BookOpen}
          variant="default"
        />
        <StatCard
          title="Today's Attendance"
          value={`${Math.round((mockStats.attendanceToday.present / mockStats.attendanceToday.total) * 100)}%`}
          subtitle={`${mockStats.attendanceToday.present} present`}
          icon={CalendarCheck}
          variant="success"
        />
        <StatCard
          title="Unpaid Fees"
          value={mockStats.unpaidFees}
          subtitle="Students with pending fees"
          icon={AlertCircle}
          variant="warning"
        />
      </div>

      {/* Tables Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
        {/* Recent Students */}
        <div>
          <h2 className="text-lg font-semibold text-foreground mb-4">Recent Enrollments</h2>
          <DataTable
            columns={studentColumns}
            data={recentStudents}
            keyExtractor={(item) => item.id}
          />
        </div>

        {/* Upcoming Fees */}
        <div>
          <h2 className="text-lg font-semibold text-foreground mb-4">Upcoming Fee Payments</h2>
          <DataTable
            columns={feeColumns}
            data={upcomingFees}
            keyExtractor={(item) => item.id}
          />
        </div>
      </div>
    </DashboardLayout>
  );
}
