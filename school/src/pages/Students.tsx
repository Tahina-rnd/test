import { useState } from 'react';
import { Plus, Search, Edit, Trash2, MoreHorizontal } from 'lucide-react';
import { DashboardLayout } from '@/components/layout/DashboardLayout';
import { PageHeader } from '@/components/common/PageHeader';
import { DataTable } from '@/components/common/DataTable';
import { StatusBadge } from '@/components/common/StatusBadge';
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
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuItem,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Label } from '@/components/ui/label';
import { useToast } from '@/hooks/use-toast';

interface Student {
  id: string;
  firstName: string;
  lastName: string;
  email: string;
  class: string;
  gender: string;
  status: 'active' | 'inactive';
  parentPhone: string;
}

// Mock data
const initialStudents: Student[] = [
  { id: '1', firstName: 'Emma', lastName: 'Johnson', email: 'emma.j@school.com', class: '10-A', gender: 'female', status: 'active', parentPhone: '+1 234 567 8901' },
  { id: '2', firstName: 'Liam', lastName: 'Smith', email: 'liam.s@school.com', class: '9-B', gender: 'male', status: 'active', parentPhone: '+1 234 567 8902' },
  { id: '3', firstName: 'Olivia', lastName: 'Brown', email: 'olivia.b@school.com', class: '11-A', gender: 'female', status: 'active', parentPhone: '+1 234 567 8903' },
  { id: '4', firstName: 'Noah', lastName: 'Williams', email: 'noah.w@school.com', class: '8-C', gender: 'male', status: 'inactive', parentPhone: '+1 234 567 8904' },
  { id: '5', firstName: 'Ava', lastName: 'Davis', email: 'ava.d@school.com', class: '12-A', gender: 'female', status: 'active', parentPhone: '+1 234 567 8905' },
  { id: '6', firstName: 'Sophia', lastName: 'Garcia', email: 'sophia.g@school.com', class: '10-B', gender: 'female', status: 'active', parentPhone: '+1 234 567 8906' },
  { id: '7', firstName: 'Mason', lastName: 'Martinez', email: 'mason.m@school.com', class: '9-A', gender: 'male', status: 'active', parentPhone: '+1 234 567 8907' },
  { id: '8', firstName: 'Isabella', lastName: 'Lopez', email: 'isabella.l@school.com', class: '11-B', gender: 'female', status: 'active', parentPhone: '+1 234 567 8908' },
];

const classes = ['8-A', '8-B', '8-C', '9-A', '9-B', '10-A', '10-B', '11-A', '11-B', '12-A', '12-B'];

export default function Students() {
  const [students, setStudents] = useState<Student[]>(initialStudents);
  const [searchQuery, setSearchQuery] = useState('');
  const [classFilter, setClassFilter] = useState<string>('all');
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [editingStudent, setEditingStudent] = useState<Student | null>(null);
  const [formData, setFormData] = useState({
    firstName: '',
    lastName: '',
    email: '',
    class: '',
    gender: '',
    parentPhone: '',
  });
  const { toast } = useToast();

  const filteredStudents = students.filter((student) => {
    const matchesSearch = 
      student.firstName.toLowerCase().includes(searchQuery.toLowerCase()) ||
      student.lastName.toLowerCase().includes(searchQuery.toLowerCase()) ||
      student.email.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesClass = classFilter === 'all' || student.class === classFilter;
    return matchesSearch && matchesClass;
  });

  const handleOpenModal = (student?: Student) => {
    if (student) {
      setEditingStudent(student);
      setFormData({
        firstName: student.firstName,
        lastName: student.lastName,
        email: student.email,
        class: student.class,
        gender: student.gender,
        parentPhone: student.parentPhone,
      });
    } else {
      setEditingStudent(null);
      setFormData({
        firstName: '',
        lastName: '',
        email: '',
        class: '',
        gender: '',
        parentPhone: '',
      });
    }
    setIsModalOpen(true);
  };

  const handleSave = () => {
    if (!formData.firstName || !formData.lastName || !formData.email || !formData.class) {
      toast({
        title: 'Error',
        description: 'Please fill in all required fields',
        variant: 'destructive',
      });
      return;
    }

    if (editingStudent) {
      setStudents(students.map((s) =>
        s.id === editingStudent.id
          ? { ...s, ...formData }
          : s
      ));
      toast({ title: 'Success', description: 'Student updated successfully' });
    } else {
      const newStudent: Student = {
        id: Date.now().toString(),
        ...formData,
        status: 'active',
      };
      setStudents([...students, newStudent]);
      toast({ title: 'Success', description: 'Student added successfully' });
    }
    setIsModalOpen(false);
  };

  const handleDelete = (id: string) => {
    setStudents(students.filter((s) => s.id !== id));
    toast({ title: 'Success', description: 'Student removed successfully' });
  };

  const columns = [
    {
      key: 'name',
      header: 'Name',
      render: (student: Student) => (
        <div>
          <div className="font-medium">{`${student.firstName} ${student.lastName}`}</div>
          <div className="text-sm text-muted-foreground">{student.email}</div>
        </div>
      ),
    },
    { key: 'class', header: 'Class' },
    { key: 'gender', header: 'Gender', render: (s: Student) => s.gender.charAt(0).toUpperCase() + s.gender.slice(1) },
    { key: 'parentPhone', header: 'Parent Contact' },
    {
      key: 'status',
      header: 'Status',
      render: (student: Student) => <StatusBadge status={student.status} />,
    },
    {
      key: 'actions',
      header: '',
      className: 'w-12',
      render: (student: Student) => (
        <DropdownMenu>
          <DropdownMenuTrigger asChild>
            <Button variant="ghost" size="icon">
              <MoreHorizontal className="w-4 h-4" />
            </Button>
          </DropdownMenuTrigger>
          <DropdownMenuContent align="end">
            <DropdownMenuItem onClick={() => handleOpenModal(student)}>
              <Edit className="w-4 h-4 mr-2" /> Edit
            </DropdownMenuItem>
            <DropdownMenuItem 
              onClick={() => handleDelete(student.id)}
              className="text-destructive"
            >
              <Trash2 className="w-4 h-4 mr-2" /> Delete
            </DropdownMenuItem>
          </DropdownMenuContent>
        </DropdownMenu>
      ),
    },
  ];

  return (
    <DashboardLayout>
      <PageHeader title="Students" subtitle="Manage student records and enrollments">
        <Button onClick={() => handleOpenModal()}>
          <Plus className="w-4 h-4 mr-2" /> Add Student
        </Button>
      </PageHeader>

      {/* Filters */}
      <div className="flex flex-col sm:flex-row gap-4 mb-6">
        <div className="relative flex-1">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
          <Input
            placeholder="Search students..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="pl-10"
          />
        </div>
        <Select value={classFilter} onValueChange={setClassFilter}>
          <SelectTrigger className="w-full sm:w-40">
            <SelectValue placeholder="All Classes" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="all">All Classes</SelectItem>
            {classes.map((cls) => (
              <SelectItem key={cls} value={cls}>{cls}</SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>

      {/* Table */}
      <DataTable
        columns={columns}
        data={filteredStudents}
        keyExtractor={(student) => student.id}
        emptyMessage="No students found"
      />

      {/* Add/Edit Modal */}
      <Dialog open={isModalOpen} onOpenChange={setIsModalOpen}>
        <DialogContent className="sm:max-w-lg">
          <DialogHeader>
            <DialogTitle>{editingStudent ? 'Edit Student' : 'Add New Student'}</DialogTitle>
          </DialogHeader>
          <div className="grid gap-4 py-4">
            <div className="grid grid-cols-2 gap-4">
              <div className="space-y-2">
                <Label htmlFor="firstName">First Name *</Label>
                <Input
                  id="firstName"
                  value={formData.firstName}
                  onChange={(e) => setFormData({ ...formData, firstName: e.target.value })}
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="lastName">Last Name *</Label>
                <Input
                  id="lastName"
                  value={formData.lastName}
                  onChange={(e) => setFormData({ ...formData, lastName: e.target.value })}
                />
              </div>
            </div>
            <div className="space-y-2">
              <Label htmlFor="email">Email *</Label>
              <Input
                id="email"
                type="email"
                value={formData.email}
                onChange={(e) => setFormData({ ...formData, email: e.target.value })}
              />
            </div>
            <div className="grid grid-cols-2 gap-4">
              <div className="space-y-2">
                <Label htmlFor="class">Class *</Label>
                <Select value={formData.class} onValueChange={(v) => setFormData({ ...formData, class: v })}>
                  <SelectTrigger>
                    <SelectValue placeholder="Select class" />
                  </SelectTrigger>
                  <SelectContent>
                    {classes.map((cls) => (
                      <SelectItem key={cls} value={cls}>{cls}</SelectItem>
                    ))}
                  </SelectContent>
                </Select>
              </div>
              <div className="space-y-2">
                <Label htmlFor="gender">Gender</Label>
                <Select value={formData.gender} onValueChange={(v) => setFormData({ ...formData, gender: v })}>
                  <SelectTrigger>
                    <SelectValue placeholder="Select gender" />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="male">Male</SelectItem>
                    <SelectItem value="female">Female</SelectItem>
                    <SelectItem value="other">Other</SelectItem>
                  </SelectContent>
                </Select>
              </div>
            </div>
            <div className="space-y-2">
              <Label htmlFor="parentPhone">Parent Phone</Label>
              <Input
                id="parentPhone"
                value={formData.parentPhone}
                onChange={(e) => setFormData({ ...formData, parentPhone: e.target.value })}
              />
            </div>
          </div>
          <DialogFooter>
            <Button variant="outline" onClick={() => setIsModalOpen(false)}>
              Cancel
            </Button>
            <Button onClick={handleSave}>
              {editingStudent ? 'Save Changes' : 'Add Student'}
            </Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </DashboardLayout>
  );
}
